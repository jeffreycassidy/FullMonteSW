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

#include <boost/range/adaptor/indexed.hpp>

#include "FullMonte.hpp"

#include "Storage/TIMOS/TIMOS.hpp"

#include "Geometry/SourceDescription.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/timer/timer.hpp>

#include "OStreamObserver.hpp"

#include "Sequence.hpp"

#include "mainloop.cpp"

#include "RandomAVX.hpp"

#ifdef STATS
#include "../Lib/Algorithm/percentile.hpp"
#endif

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
	bool meshstats=false;
}

using namespace std;

typedef std::tuple<
		LoggerEventMT,
		LoggerConservationMT,
		LoggerSurface<QueuedAccumulatorMT<double>>,
		LoggerVolume<QueuedAccumulatorMT<double>>
#ifdef TRACE_ENABLE
		,LoggerMemTraceMT
#endif
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

void mesh_stats(const TetraMesh& M);

int main(int argc,char **argv)
{
    string fn_materials,fn_sources,fn_mesh;
    vector<Material> materials;
    string prefix;

    vector<string> source_strs;

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
        ("stats,S","Calculate/display mesh statistics")
        ("source",po::value<vector<string>>(&source_strs),"Source strings")
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
    else
    	cfg.Npackets=0;

    if(vm.count("threads"))
    	opts.Nthreads=__cmdline_opts::Nthread;

    if(vm.count("rngseed"))
    	globalopts::seeds = __cmdline_opts::seeds.as_vector();
    else
    	globalopts::seeds.push_back(1);

    globalopts::meshstats = vm.count("stats");

	opts.timerinterval=globalopts::timerinterval;

    // apply options
    if (vm.count("help"))
    {
        cout << cmdline << endl;
        return -1;
    }
	cout << endl;

	if (vm.count("prefix") || vm.count("input"))
	{
		// fn_mesh will be set if input is specified
		if (!vm.count("input"))
			fn_mesh = prefix+".mesh";

		cout << "Loading mesh from " << fn_mesh << endl;

		TIMOS::Mesh M = TIMOS::parse_mesh(fn_mesh);

		cout << "  TIMOS mesh read: " << M.P.size() << " points, " << M.T.size() << " tetras" << endl;

		std::vector<Point<3,double>> P(M.P.size()+1);
		std::vector<TetraByPointID> T(M.T.size()+1);
		std::vector<unsigned> T_m(M.T.size()+1);

		P[0] = Point<3,double>();
		T[0] = TetraByPointID();
		T_m[0] = 0;

		for(unsigned i=0; i< M.P.size(); ++i)
			P[i+1] = Point<3,double>(M.P[i]);

		for(unsigned i=0; i< M.T.size(); ++i)
		{
			T[i+1] = M.T[i].IDps;
			T_m[i+1] = M.T[i].region;
		}

		geom.mesh=TetraMesh(P, T, T_m);

	}

	if (vm.count("prefix") || vm.count("sourcefile") || vm.count("source"))
	{
		if (vm.count("source"))
		{
			cout << "Using command-line specified sources: " << endl;
			for(const string& s : source_strs)
			{
				cout << "  " << s << endl;
				geom.sources.push_back(parse_string(s));
			}
		}
		else
		{
			if (!vm.count("sourcefile"))
				fn_sources=prefix+".source";

			std::vector<TIMOS::Source> src = TIMOS::parse_sources(fn_sources);
			geom.sources.clear();

			for(const TIMOS::Source& s : src)
			{
				unsigned IDf=-1;
				switch(s.type)
				{
				case TIMOS::Source::Types::Face:
					geom.sources.push_back(new FaceSourceDescription(IDf,s.w));
					break;

				case TIMOS::Source::Types::PencilBeam:
					geom.sources.push_back(new PencilBeamSourceDescription(
							Point<3,double>(s.details.pencilbeam.pos),
							UnitVector<3,double>(s.details.pencilbeam.dir.data()),
							s.w,
							s.details.pencilbeam.tetID));
					break;
				case TIMOS::Source::Types::Point:
					geom.sources.push_back(new IsotropicPointSourceDescription(
							Point<3,double>(s.details.point.pos.data()),
							(double)s.w));
					break;
				case TIMOS::Source::Types::Volume:
					geom.sources.push_back(new VolumeSourceDescription(s.details.vol.tetID,s.w));
					break;
				default:
					assert(0);
				}
			}
		}
	}

	if (vm.count("prefix") || vm.count("matfile"))
	{
		if (!vm.count("matfile"))
			fn_materials = prefix+".opt";

		TIMOS::Optical opt = TIMOS::parse_optical(fn_materials);

		geom.mats.clear();
		geom.mats.emplace_back(0.0,0.0,0.0,opt.n_ext,0.0,opt.matched);
		boost::copy(
			opt.mat | boost::adaptors::transformed(
					std::function<Material(TIMOS::Optical::Material)>([&opt](TIMOS::Optical::Material m)
							{ return Material(m.mu_a,m.mu_s,m.g,m.n,0.0,opt.matched); })),
			std::back_inserter(geom.mats));
	}

	cout << "Sources: " << endl;
	for(const auto& s : geom.sources)
		cout << *s << endl;

	cout << "Materials: " << endl;
	for (const auto& m : geom.mats)
		cout << m << endl;

	vector<unsigned> hist;
	for(unsigned i=0;i<=geom.mesh.getNt(); ++i)
	{
		unsigned mat = geom.mesh.getMaterial(i);
		if (mat >= hist.size())
			hist.resize(mat+1);
		hist[mat]++;
	}

	cout << "Material | Tetra Count" << endl;
	for(auto h : hist | boost::adaptors::indexed(0U))
		cout << h.index() << ": " << h.value() << endl;

	SourceEmitter<RNG_SFMT_AVX>* S = SourceEmitterFactory<RNG_SFMT_AVX>(geom.mesh,geom.sources);
	cout << "Source emitters: " << endl << *S << endl;

//	if (globalopts::meshstats)
//	{
//		mesh_stats(geom.mesh);
//	}

	if (cfg.Npackets == 0)
		return 0;

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

boost::timer::cpu_times runSimulation(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs)
{
	// Set up logger
    LoggerType logger = make_tuple(
    		LoggerEventMT(),
    		LoggerConservationMT(),
    		LoggerSurface<QueuedAccumulatorMT<double>>(geom.mesh,1<<10),
    		LoggerVolume<QueuedAccumulatorMT<double>>(geom.mesh,1<<10)

#ifdef TRACE_ENABLE
    		,LoggerMemTraceMT("tetra")
#endif
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

    for(const LoggerResults * lr : results)
    {
    	if (!lr)
    		cout << "Result pointer NULL" << endl;
    	else if (const VolumeArray<double>* p = dynamic_cast<const VolumeArray<double>*>(lr))
    	{
    		cout << "Writing volume energy to file" << endl;
    		ofstream os("energy.volume.out");
    		os << "# per-element absorbed energy" << endl;
    		for(double e : p->absorbed_energy())
    			os << e << endl;

            vector<double> E = p->absorbed_energy();

    		os.close();
    		os.open("fluence.volume.out");
    		os << "# per-element volume fluence" << endl;

    		double E_zero=0.0,E_trans=0.0;

    		for(unsigned i=0;i<E.size(); ++i)
    		{
    			double phi = E[i] == 0 ? 0 : E[i] / geom.mesh.getTetraVolume(i) / geom.mats[geom.mesh.getMaterial(i)].getMuA();
    			os << phi << endl;

    			if (geom.mesh.getTetraVolume(i) == 0)
    				E_zero += E[i];
    			if (geom.mats[geom.mesh.getMaterial(i)].getMuA()==0)
    				E_trans += E[i];
    		}

    		cout << "Disposed of " << E_zero << " energy in zero-volume elements" << endl;
    		cout << "Disposed of " << E_trans << " energy in transparent elements" << endl;
    	}
    	else if (const SurfaceArray<double>* p = dynamic_cast<const SurfaceArray<double>*>(lr))
    	{
    		cout << "Writing surface energy to file" << endl;
    		ofstream os("energy.surface.out");
    		os << "# per-element emitted energy" << endl;
    		for (double e : p->emitted_energy())
    			os << e << endl;
    	}
    	else
    		cout << "Not saving results of type '" << lr->getTypeString() << '\'' << endl;
    }

    return elapsed;
}

#ifdef STATS
void mesh_stats(const TetraMesh& M)
{
	vector<double> V(M.getNt(),0.0);

	for(unsigned i=0;i<M.getNt();++i)
		V[i] = M.getTetraVolume(i);

	{
		vector<pair<float,double>> Vp = percentile(V);

		ofstream os("element_volumes_by_count.hist.out");

		for(const pair<float,double>& v : Vp)
			os << v.first << ' ' << v.second << endl;


		// display specified histogram points
		cout << "% of elements with size less than" << endl;
		vector<double> p_display;
		for(unsigned i=0;i<11;++i)
			p_display.push_back(0.1*i);

		for(double p : p_display)
		{
			auto it = boost::lower_bound(Vp, p, [](pair<float,double> t,double v){ return t.first < v; });
			cout << it->first << ' ' << it->second << endl;
		}
	}

	{
		vector<pair<float,double>> Vp = percentile(V,identity(),identity(),identity());

		ofstream os("element_volumes_by_total_volume.hist.out");

		for(const pair<float,double>& v : Vp)
			os << v.first << ' ' << v.second << endl;

		// display specified histogram points
		cout << "% of volume held in elements of size less than" << endl;
		vector<double> p_display;
		for(unsigned i=0;i<11;++i)
			p_display.push_back(0.1*i);

		for(double p : p_display)
		{
			auto it = boost::lower_bound(Vp, p, [](pair<float,double> t,double v){ return t.first < v; });
			if (it == Vp.end())
				--it;
			cout << it->first << ' ' << it->second << endl;
		}
	}
}

#endif
