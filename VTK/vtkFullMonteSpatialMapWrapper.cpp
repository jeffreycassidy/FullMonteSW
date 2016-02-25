/*
 * vtkFullMonteSpatialMapWrapper.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>

#include "vtkFullMonteSpatialMapWrapper.h"

template<class VTKArrayT,typename Value,typename Index>vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::vtkFullMonteSpatialMapWrapper()
{
	m_vtkArray=VTKArrayT::New();
}

template<class VTKArrayT,typename Value,typename Index>const SpatialMapBase<Value,Index>*
	vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::source() const
{
	return m_source;
}

template<class VTKArrayT,typename Value,typename Index>void
	vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::source(const SpatialMapBase<Value,Index>* s)
{
	m_source=s;
}

template<class VTKArrayT,typename Value,typename Index>void
	vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::filter(FilterBase<Index>* f)
{
	m_filter=f;
}

template<class VTKArrayT,typename Value,typename Index>FilterBase<Index>*
	vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::filter() const
{
	return m_filter;
}



template<class VTKArrayT,typename Value,typename Index>void vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::update()
{
	assert(m_vtkArray);
	getVTKArray(*m_source,m_vtkArray);
	Modified();
}

template<class VTKArrayT,typename Value,typename Index>VTKArrayT* vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::array() const
{
	return m_vtkArray;
}


// Specific VTK instances with explicit template instantiations
typedef vtkFullMonteSpatialMapWrapper<float,unsigned> vtkFullMonteSpatialMapWrapperFU;

template<class VTKArrayT,typename Value,typename Index>vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>*
	vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>::New()
{
	return new vtkFullMonteSpatialMapWrapper<VTKArrayT,Value,Index>;
}


template class vtkFullMonteSpatialMapWrapper<vtkFloatArray,float,unsigned>;

