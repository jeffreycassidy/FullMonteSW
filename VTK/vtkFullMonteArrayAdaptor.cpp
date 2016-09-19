/*
 * vtkFullMonteArrayAdaptor.cpp
 *
 *  Created on: Aug 12, 2016
 *      Author: jcassidy
 */

#include "vtkFullMonteArrayAdaptor.h"


#include <vtkFloatArray.h>
#include <vtkAbstractArray.h>
#include <vtkObjectFactory.h>

#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>

#include <string>
#include "SwigWrapping.hpp"

using namespace std;

vtkFullMonteArrayAdaptor::vtkFullMonteArrayAdaptor()
{
	m_vtkArray = vtkFloatArray::New();
}

vtkFullMonteArrayAdaptor::~vtkFullMonteArrayAdaptor()
{
	m_vtkArray->Delete();
}

void vtkFullMonteArrayAdaptor::source(const char *mptr)
{
	SwigPointerInfo pInfo = readSwigPointer(mptr);
	string type(pInfo.type.first, pInfo.type.second-pInfo.type.first);

	if (pInfo.p)
	{
		if (type == "VolumeAbsorbedEnergyDensityMap")
			source(static_cast<const VolumeAbsorbedEnergyDensityMap*>(pInfo.p));
		else if (type == "VolumeFluenceMap")
			source(static_cast<const VolumeFluenceMap*>(pInfo.p));
		else if (type == "SurfaceFluenceMap")
			source(static_cast<const SurfaceFluenceMap*>(pInfo.p));
		else
		{
			cout << "ERROR: SWIG pointer '" << mptr << "' is not a VolumeAbsorbedEnergyDensityMap" << endl;
			source((const VolumeFluenceMap*)nullptr);
		}
	}
	else
	{
		cout << "ERROR: Failed to convert SWIG pointer '" << mptr << "'" << endl;
		source((const VolumeFluenceMap*)nullptr);
	}
}

void vtkFullMonteArrayAdaptor::source(const SurfaceFluenceMap* phi)
{
	m_type=SurfaceFluence;
	m_fullMonteFluenceS=phi;
	update();
}

void vtkFullMonteArrayAdaptor::source(const VolumeFluenceMap* phi)
{
	m_type=Fluence;
	m_fullMonteFluence=phi;
	update();
}

void vtkFullMonteArrayAdaptor::source(const VolumeAbsorbedEnergyDensityMap* E)
{
	m_type=Energy;
	m_fullMonteArray=E;
	update();
}

void vtkFullMonteArrayAdaptor::update()
{
	switch(m_type)
	{
	case Energy:
		m_vtkArray->SetNumberOfTuples((*m_fullMonteArray)->dim()); break;
	case Fluence:
		m_vtkArray->SetNumberOfTuples((*m_fullMonteFluence)->dim()); break;
	case SurfaceFluence:
		m_vtkArray->SetNumberOfTuples((*m_fullMonteFluenceS)->dim()); break;
	default:
		throw std::logic_error("Unexpected array type");
	}

	vtkFloatArray* arrayF = vtkFloatArray::SafeDownCast(m_vtkArray);

	if (!m_vtkArray)
		throw std::logic_error("Null pointer for m_vtkArray in vtkFullMonteArrayAdaptor::update()");

	if (m_type == Energy)
	{
		for(const auto i : (*m_fullMonteArray)->nonzeros())
			arrayF->SetValue(i.first,i.second);

		m_vtkArray->SetName("Absorbed Energy Density J/cm3");

	}
	else if (m_type == SurfaceFluence)
	{
		for(const auto i : (*m_fullMonteFluenceS)->nonzeros())
			arrayF->SetValue(i.first,i.second);

		m_vtkArray->SetName("Fluence J/cm2");
	}
	else if (m_type == Fluence)	{
		for(const auto i : (*m_fullMonteFluence)->nonzeros())
			arrayF->SetValue(i.first,i.second);

		m_vtkArray->SetName("Fluence J/cm2");
	}
	else
		throw std::logic_error("Invalid fluence type in vtkFullMonteArrayAdaptor");

	Modified();
}

vtkAbstractArray* vtkFullMonteArrayAdaptor::result()
{
	return m_vtkArray;
}

vtkStandardNewMacro(vtkFullMonteArrayAdaptor)

