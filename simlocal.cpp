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
#include "LoggerMemTrace.cpp"

#include "FullMonte.hpp"

#include "SourceDescription.hpp"
#include "io_timos.hpp"
#include <signal.h>
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/timer/timer.hpp>

#include "OStreamObserver.hpp"

#include "Sequence.hpp"

#include "mainloop.cpp"

#include "RandomAVX.hpp"

namespace po=boost::program_options;

namespace globalopts {
	// optional run config overrides
	boost::optional<double> prwin;
	boost::optional<double> wmin;
	boost::optional<unsigned long long> Npkt;
	boost::optional<unsigned> randseed;
	boost::optional<unsigned> Nthread;

	vector<unsigned> seeds;

	// plain ol' options
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

boost::timer::cpu_times runSimulation(const SimGeometry& sim,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs_);

void banner()
{
    cout << "FullMonte v0.0" << endl;
    cout << "(c) Jeffrey Cassidy, 2014" << endl;
    cout << endl;
}

namespace __cmdline_opts {
    	unsigned long long Npkt;
    	unsigned Nthread;
    	double prwin;
    	double wmin;
    	Sequence<unsigned> seeds;
    }

boost::timer::cpu_times runSimulation(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs);

int main(int argc,char **argv)
{
    signal(SIGHUP,SIG_IGN);
    string fn_materials,fn_sources,fn_mesh;
    vector<Material> materials;
    string prefix;

    banner();

    // define command-line options
    po::options_description cmdline("Command-line options");
    po::positional_options_description pos;
    pos.add("input",1).add("materials",1).add("sourcefile",1);

    cmdline.add_options()
        ("help,h","Display option help")
        ("input,i",po::value<string>(&fn_mesh),"Input file")
        ("N,N",po::value<unsigned long long>(&__cmdline_opts::Npkt),"Number of packets")
        ("sourcefile,s",po::value<string>(&fn_sources),"Source location file (TIM-OS .source type)")
        ("materials,m",po::value<string>(&fn_materials),"Materials file (TIM-OS .opt type)")
        ("prefix,x",po::value<string>(&prefix),"File prefix for mesh/source/materials file")
        ("rngseed,r",po::value<Sequence<unsigned>>(&__cmdline_opts::seeds),"RNG seed (int)")
        ("threads,t",po::value<unsigned>(&__cmdline_opts::Nthread),"Thread count")
        ("timer,T",po::value<double>(&globalopts::timerinterval),"Timer interval (seconds, 0=no timer)")
        ("wmin,w",po::value<double>(&__cmdline_opts::wmin),"Minimum weight for roulette")
        ("prwin,p",po::value<double>(&__cmdline_opts::prwin),"Probability of winning roulette")
        ;

    // parse options (if an option is already set, subsequent store() calls will not overwrite
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc,argv).options(cmdline).positional(pos).run(),vm);
        po::notify(vm);
    }
    catch (po::error& e){
        cerr << "Caught an exception in options processing: " << e.what() << endl;
        cout << cmdline << endl;
        return -1;
    }

    RunOptions opts;
    SimGeometry geom;
    RunConfig cfg;

    if(vm.count("wmin"))
    	cfg.wmin=__cmdline_opts::wmin;

    if(vm.count("prwin"))
    	cfg.prwin=__cmdline_opts::prwin;

    if(vm.count("N"))
    	cfg.Npackets=__cmdline_opts::Npkt;

    if(vm.count("threads"))
    	opts.Nthreads=__cmdline_opts::Nthread;

    if(vm.count("rngseed"))
    	globalopts::seeds = __cmdline_opts::seeds.as_vector();
    else
    	globalopts::seeds.push_back(1);

	opts.timerinterval=globalopts::timerinterval;

    // apply options
    if (vm.count("help"))
    {
        cout << cmdline << endl;
        return -1;
    }
	cout << endl;

	if (vm.count("prefix"))
	{
		if (vm.count("input")||vm.count("sourcefile")||vm.count("materials"))
			cout << "NOTE: --prefix option supersedes specification of input, sourcefile, and materials" << endl;
		geom.mesh=TetraMesh(prefix+".mesh",TetraMesh::MatlabTP);
		geom.mats=readTIMOSMaterials(prefix+".opt");
		geom.sources=readTIMOSSource(prefix+".source");
	}
	else if(!vm.count("input"))
	{
		cerr << "Input mesh required" << endl;
		return -1;
	}
	else if (!vm.count("sourcefile"))
	{
		cerr << "Sources required" << endl;
		return -1;
	}
	else if (!vm.count("materials"))
	{
		cerr << "Materials required" << endl;
		return -1;
	}
	else {
		// load the problem def
		geom.mesh=TetraMesh(vm["input"].as<string>(),TetraMesh::MatlabTP);
		geom.mats=readTIMOSMaterials(vm["materials"].as<string>());
		geom.sources=readTIMOSSource(vm["sourcefile"].as<string>());
	}

	try {


	for(unsigned seed : globalopts::seeds)
	{
		globalopts::randseed.reset(seed);

		OStreamObserver os_obs(cout);

		vector<Observer*> obs;
		obs.push_back(&os_obs);

		boost::timer::cpu_times e = runSimulation(geom,cfg,opts,obs);
	}
	}
	catch (std::exception& e)
	{
		cerr << "Failed with an exception: " << e.what() << endl;
		return -1;
	}
	cout << endl;
	return 0;
}

// run based on a case ID
boost::timer::cpu_times runSimulation(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs)
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
    	usleep(usecs);
    }
    while(!man.done());

    boost::timer::cpu_times elapsed = man.finish_async();
    vector<const LoggerResults*> results = man.getResults();

    return elapsed;
}
