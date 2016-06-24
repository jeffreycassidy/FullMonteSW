/*
 * TetraVolumeKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_

#include "TetraMCKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <boost/align/aligned_alloc.hpp>

#include <FullMonteSW/Kernels/Software/Logger/Logger.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerVolume.hpp>


#include <FullMonteSW/OutputTypes/SpatialMapBase.hpp>
#include <FullMonteSW/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>


class TetraVolumeKernel : public TetraMCKernel<RNG_SFMT_AVX> {
public:
	typedef std::tuple<
				MultiThreadWithIndividualCopy<LoggerEvent>,
				MultiThreadWithIndividualCopy<LoggerConservation>,
				LoggerVolume<QueuedAccumulatorMT<double> >
				>
				Logger;

	typedef std::tuple<
			MultiThreadWithIndividualCopy<LoggerEvent>::ThreadWorker,
			MultiThreadWithIndividualCopy<LoggerConservation>::ThreadWorker,
			LoggerVolume<QueuedAccumulatorMT<double>>::ThreadWorker> Worker;

	TetraVolumeKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX>(mesh)
	{
		get<2>(m_logger).resize(mesh->getNt()+1);
		get<2>(m_logger).qSize(1<<14);
	}


	/// Convenience function
	VolumeFluenceMap getVolumeFluence() const;

private:
	virtual ThreadedMCKernelBase::Thread* makeThread() override
	{
		void *p = boost::alignment::aligned_alloc(32,sizeof(TetraMCKernel<RNG_SFMT_AVX>::Thread<Worker>));

		if (!p)
			throw std::bad_alloc();

		// create the thread-local state
		typename TetraMCKernel<RNG_SFMT_AVX>::Thread<Worker>* t = new TetraMCKernel<RNG_SFMT_AVX>::Thread<Worker>(*this,get_worker(m_logger));

		// seed its RNG
		t->seed(TetraMCKernel<RNG_SFMT_AVX>::getUnsignedRNGSeed());

		return t;
	}

	virtual void prestart() override {}
	virtual void postfinish() override;

	Logger m_logger;
};



#endif /* KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_ */

