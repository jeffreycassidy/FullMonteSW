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

#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>

#include "Logger/BaseLogger.hpp"

#include "Logger/VolumeAbsorptionScorer.hpp"
#include "Logger/SurfaceExitScorer.hpp"
#include "Logger/ConservationScorer.hpp"
#include "Logger/EventScorer.hpp"
#include "Logger/DirectedSurfaceScorer.hpp"

typedef std::tuple<
		EventScorer,
		ConservationScorer,
		VolumeAbsorptionScorer,
		SurfaceExitScorer,
		DirectedSurfaceScorer>
		SVScorer;

class TetraSVKernel : public TetraMCKernel<RNG_SFMT_AVX,SVScorer>
{
public:
	TetraSVKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX,SVScorer>(mesh)
	{
	}

	typedef RNG_SFMT_AVX RNG;

	const EventScorer& 				eventScorer() 			const { return get<0>(m_scorer); }
	const ConservationScorer& 		conservationScorer() 	const { return get<1>(m_scorer); }
	const VolumeAbsorptionScorer& 	volumeScorer() 			const { return get<2>(m_scorer); }
	const SurfaceExitScorer& 		surfaceScorer() 		const { return get<3>(m_scorer); }
	const DirectedSurfaceScorer&	directedSurfaceScorer()	const { return get<4>(m_scorer); }

private:
	EventScorer& 				eventScorer()			{ return get<0>(m_scorer); }
	ConservationScorer& 		conservationScorer() 	{ return get<1>(m_scorer); }
	VolumeAbsorptionScorer& 	volumeScorer()			{ return get<2>(m_scorer); }
	SurfaceExitScorer& 			surfaceScorer() 		{ return get<3>(m_scorer); }
	DirectedSurfaceScorer&		directedSurfaceScorer()	{ return get<4>(m_scorer); }


	virtual void prepareScorer() override;
	virtual void prestart() override;
	virtual void postfinish() override;
};



#endif /* KERNELS_SOFTWARE_TETRASVKERNEL_HPP_ */

