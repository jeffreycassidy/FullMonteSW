/*
 * TetraSurfaceKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include <FullMonte/Kernels/Software/TetraSurfaceKernel.hpp>
#include "TetraMCKernelThread.hpp"

ThreadedMCKernelBase::Thread* TetraSurfaceKernel::makeThread()
{
	// create the thread-local state
	Thread<Worker>* t = new TetraMCKernel<RNG>::Thread<Worker>(*this,get_worker(m_logger));

	// seed its RNG
	t->seed(getUnsignedRNGSeed());

	return t;
}
