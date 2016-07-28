/*
 * vtkFullMonteSpatialMapWrapper.h
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#ifndef VTK_VTKFULLMONTESPATIALMAPWRAPPERFU_H_
#define VTK_VTKFULLMONTESPATIALMAPWRAPPERFU_H_

template<class Index>class FilterBase;

class vtkFloatArray;

#include <FullMonteSW/VTK/SwigWrapping.hpp>


#include <vtkObject.h>
#include <FullMonteSW/OutputTypes/SpatialMapBase.hpp>
#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>

#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>

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


class vtkFullMonteSpatialMapWrapperFU : public vtkObject
{
public:
	vtkTypeMacro(vtkFullMonteSpatialMapWrapperFU,vtkObject)

	static vtkFullMonteSpatialMapWrapperFU* New();

	void 			update();			///< Update the values from the original source
	vtkFloatArray*	array() const;		///< Return the VTK array

	///< Get/set underlying spatial map
	void 									source(const SpatialMapBase<float,unsigned>* m);
	const SpatialMapBase<float,unsigned>*	source()										const;

	void source(const char* str)
	{
		const auto ptrInfo = readSwigPointer(str);

		const std::string type(ptrInfo.type.first,ptrInfo.type.second-ptrInfo.type.first);

		std::cout << "SWIG pointer type conversion for type '" << type << "'" << std::endl;

		if (type == "SurfaceFluenceMap")
			source(static_cast<SurfaceFluenceMap*>(readSwigPointer(str).p));
		else if (type == "VolumeFluenceMap")
			source(static_cast<VolumeFluenceMap*>(readSwigPointer(str).p));
		else
			std::cout << " *** ERROR: type conversion failed" << std::endl;
	}

	void source(const VolumeFluenceMap* phi)	{ std::cout << "vtkFullMonteSpatialMapWrapperFU::source(const VolumeFluenceMap*)"  << std::endl; source(phi->get()); }
	void source(const SurfaceFluenceMap* phi)	{ std::cout << "vtkFullMonteSpatialMapWrapperFU::source(const SurfaceFluenceMap*)" << std::endl; source(phi->get()); }

	///< Get/set the filter (null -> no filter)
	void							filter(FilterBase<unsigned>* pred);
	FilterBase<unsigned>*			filter()										const;

protected:
	vtkFullMonteSpatialMapWrapperFU();

private:
	const SpatialMapBase<float,unsigned>*	m_source;	///< The source array

	FilterBase<unsigned>	*m_filter=nullptr;			///< Index filter
	vtkFloatArray			*m_vtkArray=nullptr;		///< The output VTK array
};



#endif /* VTK_VTKFULLMONTESPATIALMAPWRAPPERFU_H_ */
