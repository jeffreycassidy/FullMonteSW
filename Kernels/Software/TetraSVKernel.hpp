/*
 * TetraSVKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRASVKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRASVKERNEL_HPP_

#include "TetraMCKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <boost/align/aligned_alloc.hpp>

#include <FullMonteSW/Kernels/Software/Logger/Logger.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerVolume.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerSurface.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerInternalSurface.hpp>

#include <FullMonteSW/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

class TriFilter;

#ifndef SWIG
extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;
#endif

class TetraSVKernel : public TetraMCKernel<RNG_SFMT_AVX> {
public:
	typedef std::tuple<
				MultiThreadWithIndividualCopy<LoggerEvent>,
				MultiThreadWithIndividualCopy<LoggerConservation>,
				LoggerVolume<QueuedAccumulatorMT<double> >,
				LoggerSurface<QueuedAccumulatorMT<double>>,
				LoggerInternalSurface<QueuedAccumulatorMT<FaceCrossingAccumulator>>
				>
				Logger;

	typedef std::tuple<
			MultiThreadWithIndividualCopy<LoggerEvent>::ThreadWorker,
			MultiThreadWithIndividualCopy<LoggerConservation>::ThreadWorker,
			LoggerVolume<QueuedAccumulatorMT<double>>::ThreadWorker,
			LoggerSurface<QueuedAccumulatorMT<double>>::ThreadWorker,
			LoggerInternalSurface<QueuedAccumulatorMT<FaceCrossingAccumulator>>::ThreadWorker> Worker;

	TetraSVKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX>(mesh)
	{
		get<2>(m_logger).resize(mesh->getNt()+1);
		get<2>(m_logger).qSize(1<<14);

		get<3>(m_logger).resize(mesh->getNf()+1);
		get<3>(m_logger).qSize(1<<14);

		get<4>(m_logger).resize(mesh->getNf()+1);
		get<4>(m_logger).qSize(1<<14);
	}

	typedef RNG_SFMT_AVX RNG;

	/// Convenience functions
	// TODO: Move out of here (duplicated from VolumeKernel/SurfaceKernel
	VolumeFluenceMap 			getVolumeFluenceMap() const;
	SurfaceFluenceMap 			getSurfaceFluenceMap() const;
	InternalSurfaceFluenceMap 	getInternalSurfaceFluenceMap() const;

	VolumeAbsorbedEnergyMap		getVolumeAbsorbedEnergyMap() const;

private:
	virtual ThreadedMCKernelBase::Thread* makeThread() override
	{
		void *p = boost::alignment::aligned_alloc(32,sizeof(TetraMCKernel<RNG>::Thread<Worker>));

		if (!p)
			throw std::bad_alloc();

		// create the thread-local state
		typename TetraMCKernel<RNG>::Thread<Worker>* t = new (p) TetraMCKernel<RNG>::Thread<Worker>(*this,get_worker(m_logger));

		// seed its RNG
		t->seed(TetraMCKernel<RNG>::getUnsignedRNGSeed());

		return t;
	}

	virtual void prestart() override
	{
		log_event(m_logger,Events::clear);
	}
	virtual void postfinish() override;

	Logger m_logger;
};



#endif /* KERNELS_SOFTWARE_TETRASVKERNEL_HPP_ */

