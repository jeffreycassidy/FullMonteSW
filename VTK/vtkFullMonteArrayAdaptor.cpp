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

#include <string>
#include "SwigWrapping.hpp"

#include <FullMonteSW/OutputTypes/AbstractSpatialMap.hpp>
#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <boost/range/adaptor/indexed.hpp>

#include <cmath>
#include <sstream>
#include <iostream>

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
		if (type == "OutputData")
			source(static_cast<const OutputData*>(pInfo.p));
		else if (type == "SpatialMapT_float_t")
			source(static_cast<const SpatialMap<float>*>(pInfo.p));
		else
		{
			cout << "ERROR: SWIG pointer '" << mptr << "' is not a VolumeAbsorbedEnergyDensityMap, actually a " << type << endl;
			source(static_cast<const OutputData*>(nullptr));
		}
	}
	else
	{
		cout << "ERROR: Failed to convert SWIG pointer '" << mptr << "'" << endl;
		source(static_cast<const OutputData*>(nullptr));
	}
}

void vtkFullMonteArrayAdaptor::source(const OutputData* D)
{
	m_fullMonteArray = D;
	update();
}


void vtkFullMonteArrayAdaptor::update()
{
	// TODO: Inefficient if array is already the right size & type
	if (m_vtkArray)
		m_vtkArray->Delete();

	if (!m_fullMonteArray)
	{
		cout << "ERROR: vtkFullMonteArrayAdaptor::update() has null m_fullMonteArray" << endl;
		return;
	}

	const AbstractSpatialMap* M = dynamic_cast<const AbstractSpatialMap*>(m_fullMonteArray);

	if (!M)
	{
		cout << "ERROR: vtkFullMonteArrayAdaptor::update() cannot downcast m_fullMonteArray to AbstractSpatialMap" << endl;
		return;
	}


	if (const SpatialMap<double>* md = dynamic_cast<const SpatialMap<double>*>(m_fullMonteArray))
	{
		m_vtkArray = vtkFloatArray::New();
		m_vtkArray->SetNumberOfTuples(md->dim());

		for(const auto d : md->values() | boost::adaptors::indexed(0U))
			static_cast<vtkFloatArray*>(m_vtkArray)->SetValue(d.index(), std::isnan(d.value()) ? 0.0 : d.value());

	}
	else if (const SpatialMap<float>* mf = dynamic_cast<const SpatialMap<float>*>(m_fullMonteArray))
	{
		m_vtkArray = vtkFloatArray::New();
		m_vtkArray->SetNumberOfTuples(mf->dim());

		for(const auto f : mf->values() | boost::adaptors::indexed(0U))
			static_cast<vtkFloatArray*>(m_vtkArray)->SetValue(f.index(), isnan(f.value()) ? 0.0f : f.value());
	}
	else
	{
		cout << "ERROR! Failed to cast result in vtkFullMonteArrayAdaptor" << endl;
		return;
	}

	stringstream ss;

	switch(M->spatialType())
	{
	case AbstractSpatialMap::Surface:
		ss << "Surface";
		break;

	case AbstractSpatialMap::Volume:
		ss << "Volume";
		break;

	case AbstractSpatialMap::Line:
		ss << "Line";
		break;

	case AbstractSpatialMap::Point:
		ss << "Point";
		break;

	default:
		ss << "(unknown-spatial-type)";
		cout << "Warning: unknown type in AbstractSpatialMap" << endl;
	}

	ss << ' ' << M->quantity()->name();

	if (!M->units())
		throw std::logic_error("No units specified in vtkFullMonteArrayAdaptor::update()");

	ss << " (" << M->units()->name() << ')';

	m_vtkArray->SetName(ss.str().c_str());

	Modified();
	m_vtkArray->Modified();
}

vtkAbstractArray* vtkFullMonteArrayAdaptor::array()
{
	return m_vtkArray;
}

vtkStandardNewMacro(vtkFullMonteArrayAdaptor)
