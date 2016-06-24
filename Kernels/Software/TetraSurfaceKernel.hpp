/*
 * TetraSurfaceKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_

#include "TetraMCKernel.hpp"
#include "RNG_SFMT_AVX.hpp"

#include <boost/align/aligned_alloc.hpp>

#include <FullMonte/Kernels/Software/Logger/LoggerTuple.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerSurface.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerInternalSurface.hpp>
#include <FullMonte/Kernels/Software/Logger/AccumulationArray.hpp>

#include <FullMonte/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

#include "TetraMCKernelThread.hpp"

#include <utility>

//extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

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
			LoggerSurface<QueuedAccumulatorMT<double>>::ThreadWorker>
	Worker;

public:
	typedef RNG_SFMT_AVX RNG;
	TetraSurfaceKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG>(mesh)
	{
		get<2>(m_logger).resize(mesh->getNf()+1);
		get<2>(m_logger).qSize(1<<14);

	}

	ThreadedMCKernelBase::Thread* makeThread() override
	{
		void *p = boost::alignment::aligned_alloc(32,sizeof(TetraMCKernel<RNG>::Thread<Worker>));

		if (!p)
			throw std::bad_alloc();

		// create the thread-local state
		Thread<Worker>* t = new (p) TetraMCKernel<RNG>::Thread<Worker>(*this,get_worker(m_logger));

		// seed its RNG
		t->seed(getUnsignedRNGSeed());

		return t;
	}

	/// Convenience function
	std::vector<double> getSurfaceFluence() const;

private:
	virtual void postfinish() override;
	virtual void prestart() override;

	Logger m_logger;
};



#endif /* KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_ */
