/*
 * TetraSVKernel.cpp
 *
 *  Created on: Feb 22, 2016
 *      Author: jcassidy
 */

#include "TetraSVKernel.hpp"
#include "TetraMCKernelThread.hpp"

#include <list>

void TetraSVKernel::prestart()
{
}

void TetraSVKernel::postfinish()
{
}

void TetraSVKernel::prepareScorer()
{
	get<2>(m_scorer).dim(mesh()->getNt()+1);
	get<3>(m_scorer).dim(mesh()->getNf()+1);
	get<4>(m_scorer).dim(mesh()->getNf()+1);
}
