/*
 * TraceKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TraceKernel_HPP_
#define KERNELS_SOFTWARE_TraceKernel_HPP_

#include "TetraMCKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <FullMonteSW/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

#include "Logger/LoggerTuple.hpp"
#include "Logger/VolumeAbsorptionScorer.hpp"
#include "Logger/SurfaceExitScorer.hpp"
#include "Logger/ConservationScorer.hpp"
#include "Logger/EventScorer.hpp"
#include "Logger/PathScorer.hpp"

typedef std::tuple<
		EventScorer,
		ConservationScorer,
		VolumeAbsorptionScorer,
		SurfaceExitScorer,
		PathScorer
		>
		TraceScorer;

class TetraTraceKernel : public TetraMCKernel<RNG_SFMT_AVX,TraceScorer>
{
public:
	TetraTraceKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX,TraceScorer>(mesh)
	{
	}
	virtual ~TetraTraceKernel(){}

	typedef RNG_SFMT_AVX RNG;

	const EventScorer& eventScorer() const { return get<0>(m_scorer); }
	const SurfaceExitScorer& surfaceScorer() const { return get<3>(m_scorer); }
	const ConservationScorer& conservationScorer() const { return get<1>(m_scorer); }
	const VolumeAbsorptionScorer& volumeScorer() const { return get<2>(m_scorer); }

	const PathScorer& pathScorer() const { return get<4>(m_scorer); }

private:
	virtual void prestart() override;
	virtual void postfinish() override;
	virtual void prepareScorer() override;
};



#endif /* KERNELS_SOFTWARE_TraceKernel_HPP_ */

