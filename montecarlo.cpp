// simulator ID for database
#ifndef DB_DEF_SIMULATOR
#define DB_DEF_SIMULATOR 2
#endif

#include "AccumulationArray.hpp"
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>
#include "progress.hpp"

#include "logger.hpp"
#include "LoggerConservation.hpp"
#include "LoggerVolume.hpp"
#include "LoggerSurface.hpp"
#include "LoggerEvent.hpp"

#include "graph.hpp"
#include "source.hpp"
#include "io_timos.hpp"
#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/timer/timer.hpp>

#include "LoggerMemTrace.cpp"

#ifdef TRACER
#include "LoggerTracer.hpp"
#endif

#include "mainloop.cpp"
#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include "fmdb.hpp"
#include <map>

#include "RandomAVX.hpp"

void writeHitMap(string fn,const map<unsigned,unsigned long long>& m);

RunResults runSimulation(PGConnection* dbconn,const TetraMesh& mesh,const vector<Material>& materials,Source*,
    unsigned IDflight,unsigned IDsuite,unsigned caseorder,unsigned long long Nk);

namespace po=boost::program_options;

/* Problem definition (things that will change the expected answer)
 *
 * geometry
 * materials
 * sources
 *
 *
 * run configuration (things that will not change the expected answer)
 *
 * packet count
 * threads
 * hosts
 * wmin (will change the variance)
 *
 */

namespace globalopts {
    double Npkt;               // number of packets
    long Nk=0;                 // number of packets as long int
    unsigned Nthread=1;
    unsigned randseed=1;
    unsigned timerinterval=1;
    string logFN("log.out");   // log filename
    string outpath(".");       // output path, defaults to working dir
    bool dbwrite=true;
    double wmin=0.00001;
    double prwin=0.1;
}

using namespace std;

void banner()
{
    cout << "FullMonte v0.0" << endl;
    cout << "(c) Jeffrey Cassidy, 2014" << endl;
    cout << endl;
}

void runSuite(PGConnection* dbconn,unsigned IDflight,unsigned IDsuite);

int main(int argc,char **argv)
{
    signal(SIGHUP,SIG_IGN);
    vector<unsigned> suites;
    vector<unsigned> cases;
    unsigned long long Nk;
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
//        ("log,l",po::value<string>(&globalopts::logFN),"Log file name")
        ("N,N",po::value<double>(&globalopts::Npkt),"Number of packets")
        ("sourcefile",po::value<string>(&fn_sources),"Source location file (TIM-OS .source type)")
        ("materials,m",po::value<string>(&fn_materials),"Materials file (TIM-OS .opt type)")
        ("rngseed,r",po::value<unsigned>(&globalopts::randseed),"RNG seed (int)")
        ("threads,t",po::value<unsigned>(&globalopts::Nthread),"Thread count")
        ("Timer,T",po::value<unsigned>(&globalopts::timerinterval),"Timer interval (seconds; 0=no timer)")
//        ("outpath,p",po::value<string>(&globalopts::outpath),"Output file path")
        ("nodbwrite","Disable database writes")
        ("flightname,f",po::value<string>(&flightname),"Flight name")
        ("flightcomm,F",po::value<string>(&flightcomm),"Flight comment")
        ("case,c",po::value<vector<unsigned> >(&cases),"Cases to run from database")
        ("suite,s",po::value<vector<unsigned> >(&suites),"Suites to run from database")
        ("wmin,w",po::value<double>(&globalopts::wmin),"Minimum weight for roulette")
        ("prwin,p",po::value<double>(&globalopts::prwin),"Probability of winning roulette")
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
            IDflight = db_startFlight(dbconn.get(),flightname,flightcomm);
            cout << "Starting flight " << IDflight << endl;
        }
        catch(PGConnection::PGConnectionException& e)
        {
            cerr << "Database exception: " << e.msg << endl;
        }
    }



    globalopts::Nk=globalopts::Npkt;

    Nk=globalopts::Npkt;

    for(vector<unsigned>::const_iterator it=suites.begin(); it != suites.end(); ++it)
        runSuite(dbconn.get(),IDflight,*it);
    for(vector<unsigned>::const_iterator it=cases.begin(); it != cases.end(); ++it)
        runCaseByID(dbconn.get(),IDflight,*it,Nk);

/*    if (vm.count("input"))
    {
        TetraMesh M(fn_mesh,TetraMesh::MatlabTP);
        vector<Material> materials = readTIMOSMaterials(fn_materials);
        vector<Source*>  sources = readTIMOSSource(fn_sources);


        SurfaceFluenceMap surf(&M);
        VolumeFluenceMap vol(&M);

//        RunResults runresults = MonteCarloLoop(Nk,surf,vol,M,materials,sources);

        surf.writeASCII("output.surf.txt");
        vol.writeASCII("output.vol.txt");
    }*/

    cout << "Flight " << IDflight << " done" << endl;
    cout << "Name: " << flightname << endl << "Comment: " << flightcomm << endl;


    return 0;
}

void runSuite(PGConnection* dbconn,unsigned IDflight,unsigned IDsuite)
{
    unsigned long long Nk;
    vector<Source*> sources;
    vector<Material> materials;

    Oid pdata_oid,tdata_oid;

    // load the suite name & info from database
    PGConnection::ResultType res = dbconn->execParams("SELECT name,comment FROM suites WHERE suiteid=$1;",
        boost::tuples::make_tuple(IDsuite));

    string name,comment;
    unpackSinglePGRow(res,boost::tuples::tie(name,comment));

    cout << "Contents of suite \"" << name << "\" (ID " << IDsuite << ")" << endl;
    cout << "Comment: " << comment << endl;

    // Get cases within the suite
    res = dbconn->execParams("SELECT cases.caseid,cases.sourcegroupid,cases.materialsetid,packets,"\
        "caseorder,meshes.pdata_oid,meshes.tdata_oid,seed,wmin,threads FROM suites_map " \
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
            Nk,
            caseorder,
            pdata_oid,
            tdata_oid,
            seed,
            wmin,
            Nthread
            ), i);

        cout << "Setting globalopts::wmin=" << wmin << " globalopts::Nthread=" << Nthread << " globalopts::randseed=" << seed << endl;

	    cout << "Packets: " << bigIntSuffix(Nk) << endl;
        globalopts::wmin = wmin;
        globalopts::Nthread = Nthread;
        globalopts::randseed = seed;

        cout << "Case ID (" << IDcase << ") with " << bigIntSuffix(Nk) << " packets" << endl;
        cout << "  Source set (" << IDsg << ") from " << sourcefn << endl;

            TetraMesh m;    // TODO: There is something very screwy in the fromBinary routine
                            // it goes into infinite loop with FourLayer if the old m persists

            m.fromBinary(dbconn->loadLargeObject(pdata_oid),dbconn->loadLargeObject(tdata_oid));

            exportSources(*dbconn,IDsg,sources);
            exportMaterials(*dbconn,IDmatset,materials);

            Source* src;

            if(sources.size() > 1)
                src = new SourceMulti(sources.begin(),sources.end());
            else
                src = sources.front();

            runSimulation(dbconn,m,materials,src,IDflight,IDsuite,caseorder,Nk);
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

// run based on a case ID
void runCaseByID(PGConnection* dbconn,unsigned IDflight,unsigned IDcase,unsigned long long Nk)
{
    unsigned IDsourcegroup,IDmaterials;
    vector<Source*> sources;
    vector<Material> materials;
    TetraMesh m;
    Oid pdata_oid,tdata_oid;

    PGConnection::ResultType res = dbconn->execParams("SELECT meshes.pdata_oid,meshes.tdata_oid,cases.sourcegroupid,cases.materialsetid FROM cases " \
        "JOIN meshes ON meshes.meshid=cases.meshid " \
        "WHERE caseid=$1",
        boost::tuples::make_tuple(IDcase));

    unpackSinglePGRow(res,boost::tuples::tie(pdata_oid,tdata_oid,IDsourcegroup,IDmaterials));

    m.fromBinary(dbconn->loadLargeObject(pdata_oid),dbconn->loadLargeObject(tdata_oid));

    exportSources(*dbconn,IDsourcegroup,sources,Nk);
    exportMaterials(*dbconn,IDmaterials,materials);

    Source *src = (sources.size() > 1 ? new SourceMulti(sources.begin(),sources.end()) : sources.front());
    src->prepare(m);

    runSimulation(dbconn,m,materials,src,IDflight,0,0,Nk);
}

// Run config
//	Processors, host, nthreads, packets
//
// Case
//	Mesh, sources, materials (possibly w modifiers?)
//
// Run results
//	Min: compute time, return code

    // get the hit-count maps
/*    ls.hitMap(surfHit);
if(surfHit.size() > 0)
{
	writeHitMap("exit.count.txt",surfHit);

	Blob b = surfHit.toBinary();
	Oid oid = dbconn->createLargeObject(b);
	dbconn->execParams("INSERT INTO resultdata(runid,datatype,data_oid,total,bytesize) VALUES ($1,$2,$3,$4,$5)",
		boost::tuples::make_tuple(runid,3,oid,0,b.getSize()));
}*/

/*#ifdef LOG_EVENT
cout << "Total launched: " << le.Nlaunch << endl;
#endif

#ifdef LOG_SURFACE
SurfaceFluenceMap surf(&mesh);
sa.fluenceMap(surf);
HitMap surfHit;
cout << "Total exited:   " << surf.getTotalEnergy() << endl;
if(globalopts::dbwrite)
    db_writeResult(dbconn,runid,surf);
#endif

#ifdef LOG_VOLUME
VolumeFluenceMap vol(&mesh);
va.fluenceMap(vol,materials);
HitMap volHit;

// log the volume hits
va.hitMap(volHit);
if(volHit.size() > 0)
{
    writeHitMap("abs.count.txt",volHit);

    Blob b = volHit.toBinary();
	Oid oid = dbconn->createLargeObject(b);
    if (globalopts::dbwrite)
	    dbconn->execParams("INSERT INTO resultdata(runid,datatype,data_oid,total,bytesize) VALUES ($1,$2,$3,$4,$5)",
    	    boost::tuples::make_tuple(runid,4,oid,0,b.getSize()));
}
cout << "Total absorbed: " << vol.getTotalEnergy() << endl;

if (globalopts::dbwrite)
    db_writeResult(dbconn,runid,vol);
#endif
*/

template<typename Logger>Logger make_multilogger(Logger&& h)
{
	return h;
}

template<typename LoggerH,typename... LoggerTs>LoggerMulti<LoggerH,LoggerTs...> make_multilogger(LoggerH&& h,LoggerTs&&... ts)
{
	return LoggerMulti<LoggerH,LoggerTs...>(std::move(h),std::forward<LoggerTs>(ts)...);
}

RunResults runSimulation(PGConnection* dbconn,const TetraMesh& mesh,const vector<Material>& materials,Source* source,
    unsigned IDflight,unsigned IDsuite,unsigned caseorder,unsigned long long Nk)
{
    source->prepare(mesh);

// DEBUG: Print materials and sources
//    cout << "Materials: " << endl;
//    unsigned i=0;
//    for(vector<Material>::const_iterator it=materials.begin(); it != materials.end(); ++it,++i)
//        cout << setw(2) << i << ": " << *it << endl;
//
//    cout << "Sources: " << endl;
//    cout << *source << endl;

    cout << "Nk=" << Nk << endl;

    unsigned runid=0;

    if (globalopts::dbwrite)
        runid = db_startRun(dbconn,".","args",IDsuite,caseorder,getpid(),IDflight);

    cout << "==== Run ID " << runid << " starting" << endl;

    auto logger = make_multilogger(
    		LoggerEventMT(),
#ifdef TRACER
    		LoggerTracerMT(),
#else
    		LoggerSurface<QueuedAccumulatorMT<double>>(mesh,1<<20),
    		LoggerVolume<QueuedAccumulatorMT<double>>(mesh,1<<20),
#endif
    		LoggerConservationMT());

    // Run it
    boost::timer::cpu_times t = MonteCarloLoop<RNG_SFMT_AVX>(Nk,logger,mesh,materials,*source);

    cout << logger << endl;

    cout << "==== Run ID " << runid << " completed" << endl;

    // Gather results
    RunResults res;
    res.Np = Nk;
    res.runid = runid;

    res.exitcode=0;
    res.t_wall=t.wall/1e9;
    res.t_user=t.user/1e9;
    res.t_system=t.system/1e9;

    // write results to database
    if(globalopts::dbwrite)
    {
        db_finishRun(dbconn,runid,res);
    }

    return res;
}

void writeHitMap(string fn,const map<unsigned,unsigned long long>& m)
{
    ofstream os(fn.c_str());
    for(map<unsigned,unsigned long long>::const_iterator it=m.begin(); it != m.end(); ++it)
        os << it->first << ' ' << it->second << endl;
    
    os.close();
}
