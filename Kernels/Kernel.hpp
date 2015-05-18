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

%template(DoubleVector) std::vector<double>;

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
%include "../Geometry/Geometry_types.i"

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

	// unit of simulation energy, measured in Joules
	void setEnergy(float E){ E_=E; }

	// length of simulation unit, measured in cm
	void setUnitsToCM(){ L_=1.0;  }
	void setUnitsToMM(){ L_=0.1; }
	void setUnitsToMetre(){ L_=100.0; }

	const LoggerResults* getResult(std::string,std::string="") const;

	// Fluence in joules per cm from simulation units
	//

	double fluenceJPerCM() const {
		return E_/(L_*L_);
	}

protected:

	std::vector<SourceDescription*>	src_;
	std::vector<std::shared_ptr<const LoggerResults>> results_;

private:

	std::vector<KernelObserver*> observers_;

	virtual void prepare_(){};
	virtual void start_(){};

	// scale
	float L_=1.0;

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
	double Etotal_=1.0;
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


class SimMCThreadBase {
public:
	virtual ~SimMCThreadBase(){}
	virtual void startAsync()=0;
	virtual bool done() const=0;

	virtual void awaitFinish()=0;

	virtual unsigned long long getSimulatedPacketCount() const=0;

};

template<class Logger>class SimMCThread : public SimMCThreadBase {
public:


};


template<class RNG>class TetraMCKernel : public MonteCarloKernelBase {
public:
	void setMesh(const TetraMesh& M){ M_=M; }

	virtual void awaitFinish()
	{
		for(auto* t : workers_)
			t->awaitFinish();
		finish_();
	}

	unsigned long long getSimulatedPacketCount() const {
		unsigned long long sum=0;
		for(const SimMCThreadBase* t : workers_)
			sum += t ? t->getSimulatedPacketCount() : 0;
		return sum;
	}


protected:

	virtual bool done() const { return boost::algorithm::all_of(workers_, [](const SimMCThreadBase* w){ return w->done(); }); }

	void prepare_sources_();
	TetraMesh M_;

	vector<SimMCThreadBase*> workers_;

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

class TetraSurfaceKernel : public TetraMCKernel<RNG_SFMT_AVX> {
public:
	TetraSurfaceKernel(){}

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

	virtual void start_() override;

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

	void finish_();

};

class TetraVolumeKernel : public TetraMCKernel<RNG_SFMT_AVX> {
public:
	TetraVolumeKernel(){}

	vector<double> getVolumeFluenceVector() const
	{
		const LoggerResults* lr = getResult("logger.results.volume.energy");
		const VolumeArray<double>& d = dynamic_cast<const VolumeArray<double>&>(*lr);

		vector<double> E = d.absorbed_energy();

		cout << "Fetched an energy vector with total value " << d.getTotal() << endl;

		double k = fluenceJPerCM()/Npkt_;

		for(unsigned i=0; i<E.size(); ++i)
		{
			assert (i <= M_.getNt());
			unsigned mat = M_.getMaterial(i);
			assert (mat < mats_.size());

			double V = M_.getTetraVolume(i), mu_a = mats_[mat].mu_a;

			if (E[i] > 0)
			{
				if (V==0)
					cout << "WARNING: Nonzero absorption in an element with zero volume" << endl;
				else if (mu_a==0)
					cout << "WARNING: Nonzero absorption in an element with zero absorption coefficient" << endl;
				else
					E[i] *= k / (V * mats_[mat].mu_a);
			}
		}
		return E;
	}


private:

	virtual void start_() override;

	typedef std::tuple<
			LoggerEventMT,
			LoggerConservationMT,
			LoggerVolume<QueuedAccumulatorMT<double>>
			>
			LoggerType;

	std::unique_ptr<LoggerType> logger_;

#ifndef SWIG
	typedef decltype(get_worker(std::declval<LoggerType&>())) LoggerWorker;
#endif

	typedef TetraMCKernelThread<LoggerWorker,RNG_SFMT_AVX> TetraMCThread;

	void finish_();

};

#endif /* KERNELS_KERNEL_HPP_ */
