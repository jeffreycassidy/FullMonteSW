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

#include <FullMonteSW/Kernels/Software/Logger/LoggerTuple.hpp>

#include <FullMonteSW/Kernels/Software/Logger/VolumeAbsorptionScorer.hpp>
#include <FullMonteSW/Kernels/Software/Logger/ConservationScorer.hpp>
#include <FullMonteSW/Kernels/Software/Logger/EventScorer.hpp>

#include <FullMonteSW/OutputTypes/SpatialMap.hpp>
#include <FullMonteSW/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>
#include "Logger/BaseLogger.hpp"

typedef std::tuple<
		EventScorer,
		ConservationScorer,
		VolumeAbsorptionScorer>
	TetraVolumeScorer;

class TetraVolumeKernel : public TetraMCKernel<
	RNG_SFMT_AVX,
	TetraVolumeScorer
	>
{
public:
	TetraVolumeKernel(const TetraMesh* mesh);

private:
	virtual void prestart() override;
	virtual void postfinish() override;

	virtual void prepareScorer() override
	{
		get<2>(m_scorer).dim(mesh()->getNt()+1);
	}
};


#endif /* KERNELS_SOFTWARE_TETRAVOLUMEKERNEL_HPP_ */

