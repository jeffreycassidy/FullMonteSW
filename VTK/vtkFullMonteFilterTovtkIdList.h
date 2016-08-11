/*
 * vtkFullMonteSpatialMapWrapper.h
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#ifndef VTK_vtkFullMonteFilterTovtkIdList_H_
#define VTK_vtkFullMonteFilterTovtkIdList_H_

template<class Index>class FilterBase;

class vtkFloatArray;

#include <FullMonteSW/VTK/SwigWrapping.hpp>

#include <vtkObject.h>

template<typename T>class FilterBase;
class TetraMesh;
class vtkIdList;

class vtkFullMonteFilterTovtkIdList : public vtkObject
{
public:
	vtkTypeMacro(vtkFullMonteFilterTovtkIdList,vtkObject)

	static vtkFullMonteFilterTovtkIdList* New();

	void 			update();			///< Update the values from the original source
	vtkIdList*		idList() const;		///< Return the VTK ID list

	void				mesh(const TetraMesh* M);
	void				mesh(const char* mstr);
	const TetraMesh*	mesh() const;

	void				filter(const FilterBase<int>* F);
	void				filter(const FilterBase<unsigned>* F);

	void 				filter(const char*);

protected:
	vtkFullMonteFilterTovtkIdList();

private:
	const FilterBase<int>*		m_filterInt=nullptr;
	const FilterBase<unsigned>*	m_filterUInt=nullptr;

	const TetraMesh*			m_mesh=nullptr;

	vtkIdList*					m_idList=nullptr;
};

#endif /* VTK_vtkFullMonteFilterTovtkIdList_H_ */
