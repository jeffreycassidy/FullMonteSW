/*
 * Kernel.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef KERNELS_KERNEL_HPP_
#define KERNELS_KERNEL_HPP_

#ifndef SWIG
#define SWIG_OPENBRACE
#define SWIG_CLOSEBRACE
#endif

#ifdef SWIG
%module FullMonteKernels_TCL

%include "std_vector.i"
%include "std_string.i"

%{
#include "Kernel.hpp"
#include "KernelObserver.hpp"
#include "Software/OStreamObserver.hpp"
%}


#else


#include <FullMonte/Kernels/Software/Logger/AccumulationArray.hpp>
#include <FullMonte/Kernels/Software/Logger/Logger.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerSurface.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerVolume.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerEvent.hpp>
#include <functional>

#include <FullMonte/Kernels/KernelObserver.hpp>

#include <FullMonte/Kernels/Software/FullMonte.hpp>
#include <FullMonte/Kernels/Software/RandomAVX.hpp>
#include <FullMonte/Kernels/Software/SourceEmitter.hpp>

#include <vector>

#include <boost/algorithm/cxx11/all_of.hpp>

#include <FullMonte/Geometry/SourceDescription.hpp>
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/SimpleMaterial.hpp>

#include <memory>

#endif

#ifdef SWIG

%template(materialvector) std::vector<SimpleMaterial>;
%template(sourcedescriptionvector) std::vector<SourceDescription*>;

%include "KernelObserver.hpp"
%include "Software/OStreamObserver.hpp"

#endif

class SourceDescription;

class Kernel {
public:
	void runSync()
	{
		prepare_();

		for(const auto o: observers_)
			o->notify_start(*this);

		start_();
		awaitFinish();
	}

	void startAsync()
	{
		prepare_();
		start_();
		cout << "Kernel has started" << endl;
	}

	virtual void awaitFinish(){};

	virtual ~Kernel(){}

	//virtual void abort()=0;
	//virtual void pause()=0;

	virtual bool done() const=0;
	virtual float getProgressFraction() const=0;

	void setSources(const vector<SourceDescription*>& S);
	void setSource(SourceDescription* s);
	void setEnergy(float E){ E_=E; }

	void setUnitsToCM(){ units_per_cm_=1.0;  }
	void setUnitsToMM(){ units_per_cm_=10.0; }

	const LoggerResults* getResult(std::string,std::string="") const;

protected:

	std::vector<SourceDescription*>	src_;
	std::vector<std::shared_ptr<const LoggerResults>> results_;

private:

	std::vector<KernelObserver*> observers_;

	virtual void prepare_(){};
	virtual void start_(){};

	// scale
	float units_per_cm_=1.0;

	// total energy launched
	float E_=1.0;

	void clearSources();
};

const LoggerResults* Kernel::getResult(const std::string typeStr,const std::string opts) const
{
	auto it = boost::find_if(results_, [&typeStr](std::shared_ptr<const LoggerResults> lr){ return lr.get() && lr->getTypeString() == typeStr; });

	if (it == end(results_))
	{
		cerr << "Failed to find results of type '" << typeStr << '\'' << endl;
		return nullptr;
	}
	else
		return it->get();
}

class MonteCarloKernelBase : public Kernel {
public:
	void setPacketCount(unsigned long long Npkt){ Npkt_ = Npkt; }
	void setMaterials(const vector<SimpleMaterial>& mats);

	void setRoulettePrWin(float prwin){ prwin_=prwin; }
	void setRouletteWMin(float wmin){ wmin_=wmin; }
	void setMaxSteps(unsigned Nstep_max){ Nstep_max_=Nstep_max; }
	void setMaxHits(unsigned Nhit_max){ Nhit_max_=Nhit_max_; }

	void setThreadCount(unsigned Nth){ Nth_=Nth; }
	void setRandSeed(unsigned rs){ rngSeed_=rs; }

	float getProgressFraction() const {
		return 100.0*double(getSimulatedPacketCount())/double(Npkt_);
	}

	virtual unsigned long long getSimulatedPacketCount() const=0;

protected:
	unsigned long long Npkt_=0;
	unsigned Nstep_max_=10000;
	unsigned Nhit_max_=10000;
	float wmin_=1e-4;
	float prwin_=0.1;
	vector<SimpleMaterial> mats_;

	vector<Material> mat_;

	void prepare_materials_()
	{
		mat_.resize(mats_.size());
		boost::copy(mats_ | boost::adaptors::transformed([](SimpleMaterial ms){ return Material(ms.mu_a,ms.mu_s,ms.g,ms.n,0,0); }), mat_.begin());
	}

	virtual void finish_(){ cerr << "Invalid invocation of MonteCarloKernelBase::finish_()" << endl; }


	unsigned rngSeed_=1;
	unsigned Nth_=8;
};

template<class Logger,class RNG>class TetraMCKernelThread;

template<class RNG>class TetraMCKernel : public MonteCarloKernelBase {
public:
	void setMesh(const TetraMesh& M){ M_=M; }

protected:

	void prepare_sources_();
	TetraMesh M_;

	void prepare_();

	std::unique_ptr<const SourceEmitter<RNG>> emitter_;

	template<class T,class U>friend class TetraMCKernelThread;
};

template<class RNG>void TetraMCKernel<RNG>::prepare_sources_()
{
	emitter_ = SourceEmitterFactory<RNG_SFMT_AVX>(M_,src_);
	cout << "Source emitter: " << endl << *emitter_ << endl;
}


//class TetraVolumeKernel : public TetraMCKernel<RNG_SFMT_AVX> {
//public:
//
//	TetraVolumeKernel(){}
//
//private:
//
//	virtual void prepare_() override;
//
//	//ThreadManager<LoggerType,RNG_SFMT_AVX> man_;
//
//	typedef std::tuple<
//		LoggerEventMT,
//		LoggerConservationMT,
//		LoggerVolume<QueuedAccumulatorMT<double>>
//		>
//		LoggerType;
//
//}

#ifdef SWIG
%template(TetraMCKernelAVX) TetraMCKernel<RNG_SFMT_AVX>;
#endif

class SimMCThread {
public:
	virtual ~SimMCThread(){}
	virtual void startAsync()=0;
	virtual bool done() const=0;

	virtual void awaitFinish()=0;

	virtual unsigned long long getSimulatedPacketCount() const=0;
};

class TetraSurfaceKernel : public TetraMCKernel<RNG_SFMT_AVX> {
public:
	TetraSurfaceKernel(){}

	virtual void awaitFinish()
	{
		for(auto* t : workers_)
			t->awaitFinish();
		finish_();
	}

	vector<double> getSurfaceFluenceVector() const
	{
		const LoggerResults* lr = getResult("logger.results.surface.energy");
		const SurfaceArray<double>& d = dynamic_cast<const SurfaceArray<double>&>(*lr);

		vector<double> E = d.emitted_energy();

		cout << "Fetched an energy vector with total value " << d.getTotal() << endl;

		for(unsigned i=0; i<E.size(); ++i)
			E[i] /= M_.getFaceArea(i);
		return E;
	}


private:
	virtual void prepare_() override;

	virtual void start_() override;

	unsigned long long getSimulatedPacketCount() const {
		unsigned long long sum=0;
		for(const SimMCThread* t : workers_)
			sum += t ? t->getSimulatedPacketCount() : 0;
		return sum;
	}

	typedef std::tuple<
			LoggerEventMT,
			LoggerConservationMT,
			LoggerSurface<QueuedAccumulatorMT<double>>
			>
			LoggerType;

	std::unique_ptr<LoggerType> logger_;

#ifndef SWIG
	typedef decltype(get_worker(std::declval<LoggerType&>())) LoggerWorker;
#endif

	typedef TetraMCKernelThread<LoggerWorker,RNG_SFMT_AVX> TetraMCThread;

	vector<SimMCThread*> workers_;

	void finish_();

	virtual bool done() const { return boost::algorithm::all_of(workers_, [](const SimMCThread* w){ return w->done(); }); }
};



//	// get pointers from results tuple

//do {
//	double usecs = 1e6*opts.timerinterval;
//	tie(completed,total) = man.getProgress();
//	cout << '\r' << "  Progress: " << completed << '/' << total << " (" << fixed << setprecision(2) << double(completed)/double(total)*100.0 << "%)" << flush;
//	usleep(usecs);
//}
//while(!man.done());

#endif /* KERNELS_KERNEL_HPP_ */
