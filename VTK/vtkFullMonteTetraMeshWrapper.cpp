/*
 * vtkFullMonteTetraMeshBaseWrapper.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkIdTypeArray.h>
#include <vtkObjectFactory.h>

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/BoundingBox.hpp>
#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>

#include "vtkFullMonteTetraMeshWrapper.h"

#include "SwigWrapping.hpp"

using namespace std;

vtkStandardNewMacro(vtkFullMonteTetraMeshWrapper)


vtkFullMonteTetraMeshWrapper::vtkFullMonteTetraMeshWrapper()
{
	m_faces=vtkCellArray::New();
}

vtkFullMonteTetraMeshWrapper::~vtkFullMonteTetraMeshWrapper()
{
	m_faces->Delete();
}

void vtkFullMonteTetraMeshWrapper::mesh(const char* mptr)
{
	SwigPointerInfo pi = readSwigPointer(mptr);

	string s(pi.type.first,pi.type.second-pi.type.first);

	cout << "vtkFullMonteTetraMeshWrapper::mesh(const char* mptr) received type " << s << " (SWIG string " << mptr << ")" << endl;
	mesh(static_cast<const TetraMesh*>(pi.p));
}

void vtkFullMonteTetraMeshWrapper::mesh(const TetraMesh* m)
{
	vtkFullMonteTetraMeshBaseWrapper::mesh(static_cast<const TetraMeshBase*>(m));
	vtkFullMonteTetraMeshWrapper::update();
}

const TetraMesh* vtkFullMonteTetraMeshWrapper::mesh() const
{
	return static_cast<const TetraMesh*>(vtkFullMonteTetraMeshBaseWrapper::mesh());
}

void vtkFullMonteTetraMeshWrapper::update()
{
	assert(m_faces);
	getVTKTriangleCells(*mesh(),m_faces);
	Modified();
}

vtkPolyData* vtkFullMonteTetraMeshWrapper::faces() const
{
	vtkPolyData *pd = vtkPolyData::New();
	pd->SetPoints(points());
	pd->SetPolys(m_faces);
	return pd;
}


/** Copy the TetraMesh faces, including the 0 element (dummy containing 0,0,0)
 *
 */

void getVTKTriangleCells(const TetraMesh& M,vtkCellArray* ca,const FilterBase<int>* F)
{
	assert(ca);

	size_t Nf=0;

	// Create triangle ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);

	for(unsigned i=0;i<=M.getNf();++i)
	{
		if (!F || (*F)(i))
		{
			FaceByPointID IDps = M.getFacePointIDs(i);
			// copy regular elements 1..Nt to tetras 1..Nt
			ids->InsertNextValue(3);
			for(unsigned k=0;k<3;++k)
			{
				ids->InsertNextValue((vtkIdType)(IDps[k]));
				assert(IDps[k] < M.getNp()+1);
			}
			++Nf;
		}
	}

	ca->SetCells(Nf, ids);
	ids->Delete();
}


#include "SwigWrapping.hpp"

template<typename T>struct SwigConversionTraits;

template<>struct SwigConversionTraits<FilterBase<int>>{ static const std::string name; };

const std::string SwigConversionTraits<FilterBase<int>>::name = "FilterBase<int>";

#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>

template<typename T,typename U>U* castFromTo(void* p)
{
	return static_cast<U*>(static_cast<T*>(p));
}

class TriFilterRegionBounds;

template<typename T>T* swigCast(const char* fromType)
{
	// grab value and type from SWIG text string, check validity (non-null)
	SwigPointerInfo i = readSwigPointer(fromType);
	const std::string t(i.type.first, i.type.second-i.type.first);

	const std::string toType = SwigConversionTraits<typename std::remove_cv<T>::type>::name;

	T* o=nullptr;

	if (t == "TriFilterRegionBounds")
		o = dynamic_cast<T*>(static_cast<TriFilterRegionBounds*>(i.p));

	std::cout << "INFO: swigCast('" << fromType << "') -> " << i.p << " [" << t << "] -> " << o << " [" << toType << "]" << std::endl;

	return o;
}

vtkIdList* vtkFullMonteTetraMeshWrapper::getTriangleIDsFromFilter(const char* s)
{
	return getTriangleIDsFromFilter(swigCast<FilterBase<int>>(s));
}

vtkIdList* vtkFullMonteTetraMeshWrapper::getTriangleIDsFromFilter(const FilterBase<int>* F)
{
	std::cout << "getTriangleIDsFromFilter(" << F << " [" << F->typeStr() << "])" << std::endl;
	if (!F)
	{
		std::cout << "ERROR: null filter passed to getTriangleIDsFromFilter" << std::endl;
		return nullptr;
	}
	else if (!mesh())
	{
		return nullptr;
	}

	vtkIdList* L = vtkIdList::New();

	for(int i=1;i<mesh()->getNf()+1;++i)
		if ((*F)(i))
			L->InsertNextId(i);

	return L;
}

