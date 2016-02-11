/*
 * TetraVolumeKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_

#include "RandomAVX.hpp"
#include "TetraMCKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <FullMonte/Kernels/Software/Logger/Logger.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerTuple.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerVolume.hpp>

#include <FullMonte/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

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

//		for(unsigned i=0; i<E.size(); ++i)
//		{
//			assert (i <= m_mesh->getNt());
//			unsigned mat = m_mesh->getMaterial(i);
//			assert (mat < mats_.size());
//
//			double V = m_mesh->getTetraVolume(i), mu_a = mat_[mat].mu_a;
//
//			if (E[i] > 0)
//			{
//				if (V==0)
//					cout << "WARNING: Nonzero absorption in an element with zero volume" << endl;
//				else if (mu_a==0)
//					cout << "WARNING: Nonzero absorption in an element with zero absorption coefficient" << endl;
//				else
//					E[i] *= k / (V * mu_a);

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



#endif /* KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_ */
