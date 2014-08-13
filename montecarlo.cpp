#include "AccumulationArray.hpp"
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>

#include "Logger.hpp"
#include "LoggerConservation.hpp"
#include "LoggerVolume.hpp"
#include "LoggerSurface.hpp"
#include "LoggerEvent.hpp"

#include "FullMonte.hpp"

#include "source.hpp"
#include "io_timos.hpp"
#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/timer/timer.hpp>

#include "Notifier.hpp"

#include "TupleStuff.hpp"

#include "LoggerMemTrace.cpp"

// Tracer stores all steps the photon takes (for illustration)
//#ifdef TRACER
//#include "LoggerTracer.hpp"
//#endif

#include "mainloop.cpp"
#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include "fmdb.hpp"
#include <map>

#include "RandomAVX.hpp"

namespace po=boost::program_options;

namespace globalopts {
	// optional run config overrides
	boost::optional<double> prwin;
	boost::optional<double> wmin;
	boost::optional<unsigned long long> Npkt;
	boost::optional<unsigned> randseed;
	boost::optional<unsigned> Nthread;

	// plain ol' options
	bool dbwrite=true;
	double timerinterval=0.2;
}

using namespace std;


typedef std::tuple<
		LoggerEventMT,
		LoggerConservationMT,
		LoggerSurface<QueuedAccumulatorMT<double>>,
		LoggerVolume<QueuedAccumulatorMT<double>>
		>
		LoggerType;


//typedef typename ThreadManager<LoggerType,RNG_SFMT_AVX>::results_type ResultsType;
typedef decltype(__get_result_tuple2(std::declval<LoggerType>())) ResultsType;

pair<boost::timer::cpu_times,ResultsType> runSimulation(const SimGeometry& sim,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs_);

void banner()
{
    cout << "FullMonte v0.0" << endl;
    cout << "(c) Jeffrey Cassidy, 2014" << endl;
    cout << endl;
}

//void runSuite(PGConnection* dbconn,unsigned IDflight,unsigned IDsuite);
void runCaseByID(PGConnection* dbconn,unsigned IDcase,unsigned IDflight);

namespace __cmdline_opts {
    	unsigned long long Npkt;
    	unsigned randseed;
    	unsigned Nthread;
    	double prwin;
    	double wmin;
    }

int main(int argc,char **argv)
{
    signal(SIGHUP,SIG_IGN);
    vector<unsigned> suites;
    vector<unsigned> cases;
    unsigned IDflight=0;
    string flightname,flightcomm;
    string fn_materials,fn_sources,fn_mesh;
    vector<Source*> sources;
    vector<Material> materials;

    banner();

    // define command-line options
    po::options_description cmdline("Command-line options");
    po::positional_options_description pos;
    pos.add("input",1).add("materials",1).add("sourcefile",1);



    cmdline.add_options()
        ("help,h","Display option help")
        ("input,i",po::value<string>(&fn_mesh),"Input file")
        ("N,N",po::value<unsigned long long>(&__cmdline_opts::Npkt),"Number of packets")
        ("sourcefile",po::value<string>(&fn_sources),"Source location file (TIM-OS .source type)")
        ("materials,m",po::value<string>(&fn_materials),"Materials file (TIM-OS .opt type)")
        ("rngseed,r",po::value<unsigned>(&__cmdline_opts::randseed),"RNG seed (int)")
        ("threads,t",po::value<unsigned>(&__cmdline_opts::Nthread),"Thread count")
        ("Timer,T",po::value<double>(&globalopts::timerinterval),"Timer interval (seconds, 0=no timer)")
        ("nodbwrite","Disable database writes")
        ("flightname,f",po::value<string>(&flightname),"Flight name")
        ("flightcomm,F",po::value<string>(&flightcomm),"Flight comment")
        ("case,c",po::value<vector<unsigned> >(&cases),"Cases to run from database")
        ("suite,s",po::value<vector<unsigned> >(&suites),"Suites to run from database")
        ("wmin,w",po::value<double>(&__cmdline_opts::wmin),"Minimum weight for roulette")
        ("prwin,p",po::value<double>(&__cmdline_opts::prwin),"Probability of winning roulette")
        ;

    cmdline.add(globalopts::db::dbopts);

    // parse options (if an option is already set, subsequent store() calls will not overwrite
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc,argv).options(cmdline).positional(pos).run(),vm);
        po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
        po::notify(vm);
    }
    catch (po::error& e){
        cerr << "Caught an exception in options processing" << endl;
        cout << cmdline << endl;
        return -1;
    }

    if(vm.count("wmin"))
    	globalopts::wmin.reset(__cmdline_opts::wmin);

    if(vm.count("prwin"))
    	globalopts::prwin.reset(__cmdline_opts::prwin);

    if(vm.count("N"))
    	globalopts::Npkt.reset(__cmdline_opts::Npkt);

    if(vm.count("rngseed"))
    	globalopts::randseed.reset(__cmdline_opts::randseed);

    if(vm.count("threads"))
    	globalopts::Nthread.reset(__cmdline_opts::Nthread);

    globalopts::dbwrite=vm.count("nodbwrite")==0;


    boost::shared_ptr<PGConnection> dbconn;

    //if (!vm.count("nodbwrite")){
        cout << "DB host: " << globalopts::db::host << endl;
        cout << "DB port: " << globalopts::db::port << endl;
        cout << "DB name: " << globalopts::db::name << endl;
    
    
        try {
            dbconn = PGConnect();
        }
        catch(PGConnection::PGConnectionException& e)
        {
            cerr << "Failed to connect with error: " << e.msg << endl;
        }
    //}

    // apply options
    if (vm.count("help"))
    {
        cout << cmdline << endl;
        return -1;
    }
	cout << endl;

    if (globalopts::dbwrite){
        try {
            //IDflight = db_startFlight(dbconn.get(),flightname,flightcomm);
            cout << "Starting flight " << IDflight << endl;
        }
        catch(PGConnection::PGConnectionException& e)
        {
            cerr << "Database exception: " << e.msg << endl;
        }
    }

    try {

    //for(vector<unsigned>::const_iterator it=suites.begin(); it != suites.end(); ++it)
        //runSuite(dbconn.get(),IDflight,*it);
    	for(vector<unsigned>::const_iterator it=cases.begin(); it != cases.end(); ++it)
    		runCaseByID(dbconn.get(),*it,IDflight);
    }
    catch (PGConnection::PGConnectionException &e)
    {
    	cerr << "Caught exception: " << e.msg << endl;
    }

    cout << "Flight " << IDflight << " done" << endl;
    cout << "Name: " << flightname << endl << "Comment: " << flightcomm << endl;

    return 0;
}

/*
void runSuite(PGConnection* dbconn,unsigned IDflight,unsigned IDsuite)
{
    SimGeometry geom;
    RunConfig 	cfg;
    RunOptions	opts;

    // load the suite name & info from database
    PGConnection::ResultType res = dbconn->execParams("SELECT name,comment FROM suites WHERE suiteid=$1;",
        boost::tuples::make_tuple(IDsuite));

    string name,comment;
    unpackSinglePGRow(res,boost::tuples::tie(name,comment));

    cout << "Contents of suite \"" << name << "\" (ID " << IDsuite << ")" << endl;
    cout << "Comment: " << comment << endl;

    // Get cases within the suite
    res = dbconn->execParams("SELECT cases.caseid,cases.sourcegroupid,cases.materialsetid,packets,"\
        "caseorder,seed,wmin,threads FROM suites_map " \
        "JOIN cases ON cases.caseid=suites_map.caseid " \
        "JOIN materialsets ON materialsets.materialsetid=cases.materialsetid "\
        "JOIN sourcegroups ON sourcegroups.sourcegroupid=cases.sourcegroupid "\
        "JOIN meshes ON meshes.meshid=cases.meshid "\
        "WHERE suites_map.suiteid=$1 ORDER BY caseorder;",boost::tuples::make_tuple(IDsuite));

    int Nr = PQntuples(res.get());
    unsigned IDcase,IDsg,IDmatset;
	unsigned long long seed;
    unsigned caseorder=0;
    string casename,meshfn,meshname,sourcefn,matfn,matname;
    double wmin;
    unsigned Nthread;

    for(int i=0;i<Nr;++i)
    {
        try {
        unpackPGRow(res,boost::tuples::tie(
            IDcase,
            IDsg,
            IDmatset,
            cfg.Npackets,
            caseorder,
            seed,
            wmin,
            Nthread
            ), i);

        cout << "Setting globalopts::wmin=" << scientific << wmin << " globalopts::Nthread=" << Nthread << " globalopts::randseed=" << seed << endl;

	    cout << "Packets: " << bigIntSuffix(cfg.Npackets) << endl;

	    opts.Nthreads=Nthread;
	    opts.randseed=seed;

        cfg.prwin=globalopts::prwin;
        cfg.wmin=wmin;

        cout << "Case ID (" << IDcase << ") with " << bigIntSuffix(cfg.Npackets) << " packets" << endl;
        cout << "  Source set (" << IDsg << ") from " << sourcefn << endl;

            geom.mesh.fromBinary(dbconn->loadLargeObject(pdata_oid),dbconn->loadLargeObject(tdata_oid));

            vector<Source*> sources;
            Source* src;

            exportSources(*dbconn,IDsg,sources);
            exportMaterials(*dbconn,IDmatset,geom.mats);

            if(sources.size() > 1)
                src = new SourceMulti(sources.begin(),sources.end());
            else
                src = sources.front();

            src->prepare(geom.mesh);


            // write record to database
            unsigned runid=0;
            if (globalopts::dbwrite)
            	runid = db_startRun(dbconn,".","args",IDsuite,caseorder,getpid(),IDflight);


            RunResults res = runSimulation(geom,cfg,opts);

            // write results to database
            if(globalopts::dbwrite)
            	db_finishRun(dbconn,runid,res);
        }
        catch(PGConnection::PGConnectionException& e)
        {
            cerr << "Database exception: " << e.msg << endl;
        }
        catch(string& s)
        {
            cerr << "Caught an exception string: " << s << endl;
        }
    }
}
*/

// DEBUG: Print materials and sources
//    cout << "Materials: " << endl;
//    unsigned i=0;
//    for(vector<Material>::const_iterator it=materials.begin(); it != materials.end(); ++it,++i)
//        cout << setw(2) << i << ": " << *it << endl;
//
//    cout << "Sources: " << endl;
//    cout << *source << endl;


// run based on a case ID
void runCaseByID(PGConnection* dbconn,unsigned IDcase,unsigned IDflight)
{
    SimGeometry geom;
    RunConfig   cfg;
    RunOptions	opts;

    unsigned IDrun;

	tie(geom,cfg,opts) = exportCaseByCaseID(dbconn,IDcase);

	//if (globalopts::dbwrite)
//		IDrun=db_startRun(dbconn,cfg,opts,IDcase,IDflight);

	// override selected variables
	if(globalopts::Npkt)
		cfg.Npackets=*globalopts::Npkt;

	if(globalopts::prwin)
		cfg.prwin=*globalopts::prwin;

	if(globalopts::randseed)
		opts.randseed=*globalopts::randseed;

	if(globalopts::Nthread)
		opts.Nthreads = *globalopts::Nthread;

	if(globalopts::wmin)
		cfg.wmin=*globalopts::wmin;

	opts.timerinterval=globalopts::timerinterval;

	cout << geom << endl << cfg << endl << opts << endl;

	vector<Observer*> obs;

	obs.push_back(new OStreamObserver(cout));

	pair<boost::timer::cpu_times,ResultsType> p = runSimulation(geom,cfg,opts,obs);
}


pair<boost::timer::cpu_times,ResultsType> runSimulation(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs)
{
	// Set up logger
    LoggerType logger = make_tuple(
    		LoggerEventMT(),
    		LoggerConservationMT(),
    		LoggerSurface<QueuedAccumulatorMT<double>>(geom.mesh,1<<10),
    		LoggerVolume<QueuedAccumulatorMT<double>>(geom.mesh,1<<10)
    		);

    // Run it
    ThreadManager<LoggerType,RNG_SFMT_AVX> man(geom,cfg,opts,logger,obs);

    man.start_async();

    unsigned long long completed,total;

    do {
    	double usecs = 1e6*opts.timerinterval;
    	tie(completed,total) = man.getProgress();
    	cout << '\r' << "  Progress: " << completed << '/' << total << " (" << fixed << setprecision(2) << double(completed)/double(total)*100.0 << "%)" << flush;
    	usleep(200000);
    }
    while(!man.done());

    boost::timer::cpu_times elapsed = man.finish_async();
    auto results = __get_result_tuple2(logger);

    return make_pair(elapsed,results);
}
