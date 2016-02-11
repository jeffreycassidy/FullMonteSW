/*
 * TetraSurfaceKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_

#include "TetraMCKernel.hpp"
#include "RandomAVX.hpp"

#include <FullMonte/Kernels/Software/Logger/LoggerTuple.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerSurface.hpp>
#include <FullMonte/Kernels/Software/Logger/AccumulationArray.hpp>

#include <FullMonte/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

#include "TetraMCKernelThread.hpp"

#include <utility>

extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

class TetraSurfaceKernel : public TetraMCKernel<RNG_SFMT_AVX>
{
private:
	typedef std::tuple<
			MultiThreadWithIndividualCopy<LoggerEvent>,
			MultiThreadWithIndividualCopy<LoggerConservation>,
			LoggerSurface<QueuedAccumulatorMT<double> >
			>
			Logger;

	typedef std::tuple<
			MultiThreadWithIndividualCopy<LoggerEvent>::ThreadWorker,
			MultiThreadWithIndividualCopy<LoggerConservation>::ThreadWorker,
			LoggerSurface<QueuedAccumulatorMT<double>>::ThreadWorker> Worker;

public:
	typedef RNG_SFMT_AVX RNG;
	TetraSurfaceKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX>(mesh)
	{
		get<2>(m_logger).resize(mesh->getNf()+1);
		get<2>(m_logger).qSize(1<<14);
	}

	ThreadedMCKernelBase::Thread* makeThread() override
	{
		// create the thread-local state
		Thread<Worker>* t = new TetraMCKernel<RNG>::Thread<Worker>(*this,get_worker(m_logger));

		// seed its RNG
		t->seed(getUnsignedRNGSeed());

		return t;
	}

private:
	void postfinish();
	void prestart();

	Logger m_logger;
};



#endif /* KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_ */
