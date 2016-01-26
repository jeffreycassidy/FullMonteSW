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

#include <FullMonte/VTK/SparseVectorVTK.h>
#include <FullMonte/OutputTypes/SparseVector.hpp>

vtkCxxRevisionMacro(SparseVectorVTK,"Rev 0.0");
vtkStandardNewMacro(SparseVectorVTK);

SparseVectorVTK::SparseVectorVTK()
{
	m_vtkFloat = vtkFloatArray::New();
	m_vtkFloat->SetNumberOfComponents(1);
}

SparseVectorVTK::~SparseVectorVTK()
{
}

void SparseVectorVTK::PrintSelf(std::ostream& os,vtkIndent indent)
{
	vtkObject::PrintSelf(os,indent);
}

void SparseVectorVTK::setInputSparseVector(const SparseVector<unsigned,float>* sv)
{
	////// Update pointer to source
	m_sv = sv;
	update();
}

void SparseVectorVTK::setInputSparseVector(const char* pStr)
{
	// kludgy, home-brewed pointer extraction
	// format is _XXXXXXXXX_p_TTTTTT where X is pointer value (little-endian hex) and T is type
	std::stringstream ss(pStr);
	std::string typeStr;
	char buf[5];
	uint64_t pval;
	ss.read(buf,1);
	ss >> std::hex >> pval;
	ss.read(buf+1,3);
	buf[4]=0;
	if (strcmp("__p_",buf))
		std::cout << "ERROR: Improperly formatted string" << std::endl;

	ss >> typeStr;
	std::cout << "typeStr='" << typeStr << "'" << std::endl;

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!
	pval = (pval >> 32) | (pval << 32);
	pval = ((pval >> 16) & 0x0000ffff0000ffffULL) | ((pval << 16) & 0xffff0000ffff0000ULL);
	pval = ((pval >> 8 ) & 0x00ff00ff00ff00ffULL) | ((pval << 8)  & 0xff00ff00ff00ff00ULL);

	m_sv = reinterpret_cast<const SparseVector<unsigned,float>*>(pval);
	update();
}

void SparseVectorVTK::update()
{
	////// Update the float arrays
	if (m_sv)
	{
		m_vtkFloat->SetNumberOfTuples(m_sv->dim());

		for(const auto iv : m_sv->dense())
		{
			assert(iv.index() < m_sv->dim());
			m_vtkFloat->SetTuple1(iv.index(), iv.value());
		}
	}
	else
		m_vtkFloat->SetNumberOfTuples(0);

	Modified();
}

// Should be OK to return a pointer since VTK requires that filters not modify their inputs
vtkFloatArray* SparseVectorVTK::GetOutput()
{
	return m_vtkFloat;
}
