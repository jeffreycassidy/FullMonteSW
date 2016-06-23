/*
 * SparseVectorVTK.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: jcassidy
 */

#include <vtkFloatArray.h>
#include <vtkObjectFactory.h>

#include <sstream>

using namespace std;

#include <FullMonteSW/VTK/SparseVectorVTK.h>
#include <FullMonteSW/OutputTypes/SpatialMapBase.hpp>
//
//vtkCxxRevisionMacro(SparseVectorVTK,"Rev 0.0");
//vtkStandardNewMacro(SparseVectorVTK);

void getVTKFloatArray(const SpatialMapBase<float,unsigned>& SV,vtkFloatArray* a)
{
	assert(a);
	a->SetNumberOfTuples(SV.dim());

	for(const auto iv : SV.dense())
	{
		assert(iv.index() < SV.dim());
		a->SetValue(iv.first, iv.second);
	}
}


//
//SparseVectorVTK::SparseVectorVTK()
//{
//	m_vtkFloat = vtkFloatArray::New();
//	m_vtkFloat->SetNumberOfComponents(1);
//}
//
//SparseVectorVTK::~SparseVectorVTK()
//{
//}
//
//void SparseVectorVTK::PrintSelf(std::ostream& os,vtkIndent indent)
//{
//	vtkObject::PrintSelf(os,indent);
//}
//
//void SparseVectorVTK::setInputSparseVector(const SparseVector<unsigned,float>* sv)
//{
//	////// Update pointer to source
//	m_sv = sv;
//	update();
//}
//
//void SparseVectorVTK::update()
//{
//
//	}
//	else
//		m_vtkFloat->SetNumberOfTuples(0);
//
//	Modified();
//}
//
//// Should be OK to return a pointer since VTK requires that filters not modify their inputs
//vtkFloatArray* SparseVectorVTK::GetOutput()
//{
//	return m_vtkFloat;
//}
