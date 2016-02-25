/*
 * vtkFullMonteSpatialMapWrapper.h
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#ifndef VTK_VTKFULLMONTESPATIALMAPWRAPPER_H_
#define VTK_VTKFULLMONTESPATIALMAPWRAPPER_H_

template<class Index>class FilterBase;

#include <vtkObject.h>
#include <FullMonte/OutputTypes/SpatialMapBase.hpp>

#include <FullMonte/Geometry/Filters/FilterBase.hpp>

/** Convert a SpatialMapBase<V,I> into a VTK array of the corresponding type */

template<typename VTKArrayT,typename Value,typename Index>void getVTKArray(const SpatialMapBase<Value,Index>& SV,VTKArrayT* a)
{
	assert(a);
	a->SetNumberOfTuples(SV.dim());

	for(const auto iv : SV.dense())
	{
		assert(iv.first < SV.dim());
		a->SetValue(iv.first, iv.second);
	}
	a->Modified();
}


template<class VTKArrayT,typename Value,typename Index=unsigned>class vtkFullMonteSpatialMapWrapper : public vtkObject
{
public:
	vtkTypeMacro(vtkFullMonteSpatialMapWrapper,vtkObject)

	static vtkFullMonteSpatialMapWrapper* New();

	void 		update();			///< Update the values from the original source
	VTKArrayT*	array() const;		///< Return the VTK array

	///< Get/set underlying spatial map
	void 								source(const SpatialMapBase<Value,Index>* m);
	const SpatialMapBase<Value,Index>*	source()										const;

	///< Get/set the filter (null -> no filter)
	void							filter(FilterBase<Index>* pred);
	FilterBase<Index>*				filter()										const;

protected:
	vtkFullMonteSpatialMapWrapper();

private:
	const SpatialMapBase<Value,Index>*	m_source;	///< The source array

	FilterBase<Index>	*m_filter=nullptr;			///< Index filter
	VTKArrayT			*m_vtkArray=nullptr;		///< The output VTK array
};





#endif /* VTK_VTKFULLMONTESPATIALMAPWRAPPER_H_ */
