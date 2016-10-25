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

#include <boost/align/aligned_alloc.hpp>

#include "Logger/LoggerTuple.hpp"
#include "Logger/VolumeAbsorptionScorer.hpp"
#include <FullMonteSW/Kernels/Software/Logger/SurfaceExitScorer.hpp>

#include <FullMonteSW/Kernels/Software/Logger/MultiThreadWithIndividualCopy.hpp>

#include <FullMonteSW/OutputTypes/PacketPositionTraceSet.hpp>
#include "Logger/BaseLogger.hpp"
#include "Logger/ConservationScorer.hpp"
#include "Logger/EventScorer.hpp"
#include "Logger/PathTracer.hpp"

typedef std::tuple<
		EventScorer,
		ConservationScorer,
		VolumeAbsorptionScorer,
		SurfaceExitScorer
//		LoggerTracerMT
		>
		TraceScorer;

class TraceKernel : public TetraMCKernel<RNG_SFMT_AVX,TraceScorer>
{
public:
	TraceKernel(const TetraMesh* mesh) :
		TetraMCKernel<RNG_SFMT_AVX,TraceScorer>(mesh)
	{
//		get<2>(m_logger).resize(mesh->getNt()+1);
//		get<2>(m_logger).qSize(1<<14);
//
//		get<3>(m_logger).resize(mesh->getNf()+1);
//		get<3>(m_logger).qSize(1<<14);
	}
	~TraceKernel(){}

	typedef RNG_SFMT_AVX RNG;

private:
	virtual void prestart() override
	{
		//log_event(m_logger,Events::clear);
	}
	virtual void postfinish() override;

	virtual void prepareScorer() override
			{

			}

	TraceScorer m_scorer;
};



#endif /* KERNELS_SOFTWARE_TraceKernel_HPP_ */

