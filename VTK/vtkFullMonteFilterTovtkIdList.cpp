/*
 * vtkFullMonteSpatialMapWrapper.cpp
 *
 *  Created on: Mar 3, 2016
 *      Author: jcassidy
 */

#include <vtkIdList.h>
#include <vtkObjectFactory.h>

#include <vtkIdList.h>

#include <cassert>

#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>

#include "vtkFullMonteFilterTovtkIdList.h"

void vtkFullMonteFilterTovtkIdList::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

void vtkFullMonteFilterTovtkIdList::mesh(const char* mstr)
{
	SwigPointerInfo p = readSwigPointer(mstr);

	if (p.p)
	{
		mesh(static_cast<const TetraMesh*>(p.p));
	}
}

const TetraMesh* vtkFullMonteFilterTovtkIdList::mesh() const
{
	return m_mesh;
}

vtkFullMonteFilterTovtkIdList::vtkFullMonteFilterTovtkIdList()
{
	m_idList=vtkIdList::New();
}

void vtkFullMonteFilterTovtkIdList::filter(const FilterBase<int>* F)
{
	m_filterInt=F;
	m_filterUInt=nullptr;
}

void vtkFullMonteFilterTovtkIdList::filter(const FilterBase<unsigned>* F)
{
	m_filterUInt=F;
	m_filterInt=nullptr;
}

void vtkFullMonteFilterTovtkIdList::filter(const char* str)
{
	SwigPointerInfo p = readSwigPointer(str);
	std::string type(p.type.first,p.type.second-p.type.first);

	cout << "vtkFullMonteFilterTovtkIdList::filter(const char*) called with '" << str << "' (type '" << type << "')" << endl;


	m_filterInt=nullptr;
	m_filterUInt=nullptr;

	if (p.p)
	{
		cout << "  Recognized as a SWIG pointer valued " << p.p << endl;
		if (type == "TriFilterRegionBounds")
			m_filterInt = static_cast<TriFilterRegionBounds*>(p.p);
		else
		{
			cout << "    Unrecognized type - just setting null for now" << endl;
		}
	}
	else
	{
		cout << "  Unknown type - maybe a vtkObject or SWIG named object?" << endl;
	}
	update();
}

void vtkFullMonteFilterTovtkIdList::update()
{
	assert(m_idList);

	m_idList->Reset();

	if (m_filterInt)
	{
		for(int i=0;i<=m_mesh->getNf();++i)
			if ((*m_filterInt)(i))
				m_idList->InsertNextId(i);
	}
	else if (m_filterUInt)
	{
		for(unsigned i=0;i<=m_mesh->getNt();++i)
			if ((*m_filterUInt)(i))
				m_idList->InsertNextId(i);

	}

	Modified();
}

vtkIdList* vtkFullMonteFilterTovtkIdList::idList() const
{
	return m_idList;
}

vtkStandardNewMacro(vtkFullMonteFilterTovtkIdList)

