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

#include <FullMonteSW/Kernels/Software/Logger/Logger.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerVolume.hpp>
#include <FullMonteSW/Kernels/Software/Logger/LoggerSurface.hpp>

#include <FullMonteSW/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

class TetraSVKernel : public TetraMCKernel<RNG_SFMT_AVX> {
public:
	typedef std::tuple<
				MultiThreadWithIndividualCopy<LoggerEvent>,
				MultiThreadWithIndividualCopy<LoggerConservation>,
				LoggerVolume<QueuedAccumulatorMT<double> >,
				LoggerSurface<QueuedAccumulatorMT<double>>
				>
				Logger;

	typedef std::tuple<
			MultiThreadWithIndividualCopy<LoggerEvent>::ThreadWorker,
			MultiThreadWithIndividualCopy<LoggerConservation>::ThreadWorker,
			LoggerVolume<QueuedAccumulatorMT<double>>::ThreadWorker,
			LoggerSurface<QueuedAccumulatorMT<double>>::ThreadWorker> Worker;

	TetraSVKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX>(mesh)
	{
		get<2>(m_logger).resize(mesh->getNt()+1);
		get<2>(m_logger).qSize(1<<14);

		get<3>(m_logger).resize(mesh->getNf()+1);
		get<3>(m_logger).qSize(1<<14);
	}

private:
	virtual ThreadedMCKernelBase::Thread* makeThread() override
	{
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



#endif /* KERNELS_SOFTWARE_TETRASVKERNEL_HPP_ */
