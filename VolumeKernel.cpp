#include <FullMonte/Kernels/Software/AccumulationArray.hpp>
#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>

#include <FullMonte/Kernels/Software/Logger.hpp>
#include <FullMonte/Kernels/Software/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/LoggerVolume.hpp>
#include <FullMonte/Kernels/Software/LoggerEvent.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <FullMonte/Kernels/Software/FullMonte.hpp>

#include "Geometry/SourceDescription.hpp"

#include <boost/timer/timer.hpp>

#include <FullMonte/Kernels/Software/OStreamObserver.hpp>

#include <FullMonte/Kernels/Software/mainloop.cpp>

#include <FullMonte/Kernels/Software/RandomAVX.hpp>


using namespace std;

std::vector<double> VolumeKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const std::vector<SourceDescription*>& src,unsigned long long Npkt,double units_per_cm,double E);

std::vector<double> VolumeKernel(const TetraMesh& mesh,const std::vector<SimpleMaterial>& mats,const IsotropicPointSourceDescription& ips,unsigned long long Npkt,double units_per_cm,double E)
{
	vector<Material> mat(mats.size());

	boost::copy(mats | boost::adaptors::transformed([](SimpleMaterial sm){ return Material(sm.mu_a,sm.mu_s,sm.g,sm.n); }), mat.begin());
	return VolumeKernel(mesh,mat,std::vector<SourceDescription*>{ (SourceDescription*)&ips },Npkt,units_per_cm,E);
}


typedef std::tuple<
		LoggerEventMT,
		LoggerConservationMT,
		LoggerVolume<QueuedAccumulatorMT<double>>
		>
		LoggerType;

std::vector<double> runSimulation(const SimGeometry& sim,const RunConfig& cfg,const RunOptions& opts,const vector<Observer*>& obs_);

std::vector<double> VolumeKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const std::vector<SourceDescription*>& src,unsigned long long Npkt,double units_per_cm,double E)
{
    RunOptions opts;
    SimGeometry geom;
    RunConfig cfg;

	opts.timerinterval=0.1;
	opts.Nhit_max=10000;
	opts.Nstep_max=100000;
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

	vector<double> phi_v = runSimulation(geom,cfg,opts,obs);

	double k = E*units_per_cm*units_per_cm/double(Npkt);

	for(double& phi : phi_v)
		phi *= k;

	return phi_v;
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

    vector<double> E_v;
    vector<double> phi_v(geom.mesh.getNt()+1);

    for(const LoggerResults * lr : results)
    {
    	if (!lr)
    		cout << "Result pointer NULL" << endl;
    	else if (const VolumeArray<double>* p = dynamic_cast<const VolumeArray<double>*>(lr))
    	{
    		E_v = p->absorbed_energy();

    		for(unsigned i=0; i<=geom.mesh.getNt(); ++i)
    		{
    			double v = geom.mesh.getTetraVolume(i);
    			double mu_a = geom.mats[geom.mesh.getMaterial(i)].getMuA();

   				phi_v[i] = E_v[i] > 0.0 ? E_v[i]/v/mu_a : 0.0;
    		}
    	}
    	else
    		cout << "Skipping results of type '" << lr->getTypeString() << '\'' << endl;
    }

    return phi_v;
}
