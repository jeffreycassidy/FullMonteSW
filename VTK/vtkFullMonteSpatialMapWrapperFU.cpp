/*
 * vtkFullMonteSpatialMapWrapper.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>

#include "vtkFullMonteSpatialMapWrapperFU.h"

vtkFullMonteSpatialMapWrapperFU::vtkFullMonteSpatialMapWrapperFU()
{
	m_vtkArray=vtkFloatArray::New();
}

const SpatialMapBase<float,unsigned>* vtkFullMonteSpatialMapWrapperFU::source() const
{
	return m_source;
}

void vtkFullMonteSpatialMapWrapperFU::source(const SpatialMapBase<float,unsigned>* s)
{
	m_source=s;
}

void
	vtkFullMonteSpatialMapWrapperFU::filter(FilterBase<unsigned>* f)
{
	m_filter=f;
}

FilterBase<unsigned>*
	vtkFullMonteSpatialMapWrapperFU::filter() const
{
	return m_filter;
}



void vtkFullMonteSpatialMapWrapperFU::update()
{
	assert(m_vtkArray);
	getVTKArray(*m_source,m_vtkArray);
	Modified();
}

vtkFloatArray* vtkFullMonteSpatialMapWrapperFU::array() const
{
	return m_vtkArray;
}

vtkStandardNewMacro(vtkFullMonteSpatialMapWrapperFU)

