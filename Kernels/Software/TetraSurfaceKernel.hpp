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

#include <FullMonte/Kernels/Software/Logger/LoggerEvent.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerConservation.hpp>
#include <FullMonte/Kernels/Software/Logger/LoggerSurface.hpp>
#include <FullMonte/Kernels/Software/Logger/AccumulationArray.hpp>

#include "TetraMCKernelThread.hpp"

#include <utility>

extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

class TetraSurfaceKernel : public TetraMCKernel<RNG_SFMT_AVX>
{
private:
	typedef std::tuple<
			LoggerEventMT,
			LoggerConservationMT,
			LoggerSurface<QueuedAccumulatorMT<double> >
			>
			Logger;

	typedef std::tuple<
			LoggerEventMT::ThreadWorker,
			LoggerConservationMT::ThreadWorker,
			LoggerSurface<QueuedAccumulatorMT<double>>::ThreadWorker> Worker;

public:
	typedef RNG_SFMT_AVX RNG;
	TetraSurfaceKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX>(mesh),
		m_logger(
				LoggerEventMT(),
				LoggerConservationMT(),
				LoggerSurface<QueuedAccumulatorMT<double>>(*mesh,1<<10))
	{}

	virtual ThreadedMCKernelBase::Thread* makeThread() override;

	vector<double> getSurfaceFluenceVector() const
	{
		const LoggerResults* lr = getResult("logger.results.surface.energy");
		const SurfaceArray<double>& d = dynamic_cast<const SurfaceArray<double>&>(*lr);

		vector<double> E = d.emitted_energy();

		cout << "Fetched an energy vector with total value " << d.getTotal() << endl;

		E[0] = 0;
		for(unsigned i=1; i<E.size(); ++i)
		{
			assert(m_mesh->getFaceArea(i) > 0);
			E[i] /= m_mesh->getFaceArea(i);
		}

		return E;
	}

private:

	Logger m_logger;
};



#endif /* KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_ */
