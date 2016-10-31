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

#include "TetraMCKernelThread.hpp"

#include "Logger/LoggerTuple.hpp"
#include "Logger/SurfaceExitScorer.hpp"
#include "Logger/ConservationScorer.hpp"
#include "Logger/EventScorer.hpp"

typedef std::tuple<
		EventScorer,
		ConservationScorer,
		SurfaceExitScorer>
		TetraSurfaceScorer;

class TetraSurfaceKernel : public TetraMCKernel<RNG_SFMT_AVX,TetraSurfaceScorer>
{
public:
	TetraSurfaceKernel(const TetraMesh* mesh);

	const EventScorer& 				eventScorer() 			const { return get<0>(m_scorer); }
	const ConservationScorer& 		conservationScorer() 	const { return get<1>(m_scorer); }
	const SurfaceExitScorer& 		surfaceScorer() 		const { return get<2>(m_scorer); }

private:
	EventScorer& 				eventScorer()			{ return get<0>(m_scorer); }
	ConservationScorer& 		conservationScorer() 	{ return get<1>(m_scorer); }
	SurfaceExitScorer& 			surfaceScorer() 		{ return get<2>(m_scorer); }

	virtual void postfinish() override;
	virtual void prestart() override;
	virtual void prepareScorer() override;

};



#endif /* KERNELS_SOFTWARE_TETRASURFACEKERNEL_HPP_ */

