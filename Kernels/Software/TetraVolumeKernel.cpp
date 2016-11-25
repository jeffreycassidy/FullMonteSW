/*
 * TetraVolumeKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include "TetraVolumeKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <FullMonteSW/OutputTypes/OutputDataSummarize.hpp>
#include <boost/range/adaptor/indexed.hpp>

#include <list>

void TetraVolumeKernel::prestart()
{
}

void TetraVolumeKernel::postfinish()
{
}

TetraVolumeKernel::TetraVolumeKernel() : TetraMCKernel<RNG_SFMT_AVX,TetraVolumeScorer>(nullptr)
{

}

TetraVolumeKernel::TetraVolumeKernel(const TetraMesh* mesh) :
	TetraMCKernel<RNG_SFMT_AVX,TetraVolumeScorer>(mesh)
{
}

