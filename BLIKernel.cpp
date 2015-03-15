#include <FullMonte/Kernels/Software/AccumulationArray.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>

#include <FullMonte/Kernels/Software/Logger.hpp>
#include <FullMonte/Kernels/Software/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/LoggerSurface.hpp>
#include <FullMonte/Kernels/Software/LoggerEvent.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <FullMonte/Kernels/Software/FullMonte.hpp>

#include "Geometry/SourceDescription.hpp"

#include <boost/timer/timer.hpp>

#include <FullMonte/Kernels/Software/OStreamObserver.hpp>

#include <FullMonte/Kernels/Software/mainloop.cpp>

#include <FullMonte/Kernels/Software/RandomAVX.hpp>


using namespace std;

std::vector<double> BLIKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const std::vector<SourceDescription*>& src,unsigned long long Npkt);

std::vector<double> BLIKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const BallSourceDescription& bsd,unsigned long long Npkt)
{
	return BLIKernel(mesh,mats,std::vector<SourceDescription*>{ (SourceDescription*)&bsd },Npkt);
}


typedef std::tuple<
		LoggerEventMT,
		LoggerConservationMT,
		LoggerSurface<QueuedAccumulatorMT<double>>
		>
		LoggerType;

std::vector<double> runSimulation(const SimGeometry& sim,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs_);

std::vector<double> BLIKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const std::vector<SourceDescription*>& src,unsigned long long Npkt)
{
    RunOptions opts;
    SimGeometry geom;
    RunConfig cfg;

	opts.timerinterval=0.1;
	opts.Nhit_max=100;
	opts.Nstep_max=10000;
	opts.Nthreads=8;
	opts.randseed=1;

	geom.mesh = mesh;
	geom.IDc=0;
	geom.mats=mats;
	geom.sources=src;

	cfg.wmin=1e-4;
	cfg.prwin=0.1;
	cfg.Npackets=Npkt;

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

	OStreamObserver os_obs(cout);

	vector<Observer*> obs;
	obs.push_back(&os_obs);

	return runSimulation(geom,cfg,opts,obs);
}

std::vector<double> runSimulation(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs)
{
	if (geom.sources.size() == 0)
	{
		cerr << "ERROR: No sources specified" << endl;
		return std::vector<double>();
	}
	// Set up logger
    LoggerType logger = make_tuple(
    		LoggerEventMT(),
    		LoggerConservationMT(),
    		LoggerSurface<QueuedAccumulatorMT<double>>(geom.mesh,1<<10)
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

    vector<double> E(geom.mesh.getNf()+1);
    vector<double> phi_s(geom.mesh.getNf()+1);

    for(const LoggerResults * lr : results)
    {
    	if (!lr)
    		cout << "Result pointer NULL" << endl;
    	else if (const SurfaceArray<double>* p = dynamic_cast<const SurfaceArray<double>*>(lr))
    	{
    		E = p->emitted_energy();
    		for(unsigned i=0; i<=geom.mesh.getNf(); ++i)
    			phi_s[i] = E[i] / geom.mesh.getFaceArea(i);
    	}
    	else
    		cout << "Skipping results of type '" << lr->getTypeString() << '\'' << endl;
    }

    return phi_s;
}
