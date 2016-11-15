/*
 * FluenceConverter.cpp
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */


#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include "EnergyToFluence.hpp"

#include <FullMonteSW/OutputTypes/DirectedSurfaceElement.hpp>

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/SimpleMaterial.hpp>
#include <FullMonteSW/OutputTypes/DirectedSurfaceElement.hpp>

#include <FullMonteSW/Geometry/Units/BaseUnit.hpp>

class EnergyToFluence::Converter
{
public:
	virtual ~Converter(){}
	virtual float operator()(std::size_t i,float x) const=0;
	//virtual double operator()(std::size_t i,double x) const=0;

	virtual DirectedSurfaceElement<float> operator()(std::size_t i,DirectedSurfaceElement<float> x) const
	{
		return DirectedSurfaceElement<float>(operator()(i,x.exit()), operator()(i,x.enter()));
	}
};

class EnergyToFluence::SurfaceConverter : public EnergyToFluence::Converter
{
public:
	SurfaceConverter(const TetraMesh* M) : m_mesh(M){}

	virtual float operator()(std::size_t i,float x) const override
	{
		return x / m_mesh->getFaceArea(unsigned(i));
	}

protected:
	const TetraMesh* m_mesh=nullptr;
};

class EnergyToFluence::DirectedSurfaceConverter : public EnergyToFluence::SurfaceConverter
{
public:
	DirectedSurfaceConverter(const TetraMesh* M) : SurfaceConverter(M){}
	~DirectedSurfaceConverter(){}

	virtual DirectedSurfaceElement<float> operator()(std::size_t i,DirectedSurfaceElement<float> x) const override
	{
		float k = 1.0 / m_mesh->getFaceArea(unsigned(i));
		return DirectedSurfaceElement<float>(
				k*x.enter(),
				k*x.exit());
	}
};

class EnergyToFluence::VolumeConverter : public EnergyToFluence::Converter
{
public:
	VolumeConverter(const TetraMesh* M,const std::vector<SimpleMaterial>* mat) :
		m_mesh(M),
		m_materials(mat){}

	virtual float operator()(std::size_t i,float x) const override
	{
		return x / (get(volume,*m_mesh,TetraMeshBase::TetraDescriptor(i)) * (*m_materials)[m_mesh->getMaterial(i)].muA());
	}

private:
	const TetraMesh* m_mesh=nullptr;
	const std::vector<SimpleMaterial>* m_materials=nullptr;
};



EnergyToFluence::EnergyToFluence()
{
}

EnergyToFluence::~EnergyToFluence()
{
}

void EnergyToFluence::source(OutputData* M)
{
	m_values=dynamic_cast<AbstractSpatialMap*>(M);
	if (!m_values)
		cout << "ERROR: EnergyToFluence::source(OutputData* M) input cannot be cast to AbstractSpatialMap" << endl;
	else
		cout << "INFO: EnergyToFluence::source(OutputData*) updated source with dim=" << m_values->dim() << endl;
}

const AbstractSpatialMap* EnergyToFluence::source() const
{
	return m_values;
}

void EnergyToFluence::update()
{
	Converter* C=nullptr;
	delete m_output;
	m_output=nullptr;

	if (!m_values)
	{
		cout << "ERROR: EnergyToFluence::results() requested but no input values" << endl;
		return;
	}

	switch(m_values->spatialType())
	{
	case AbstractSpatialMap::Surface:
		if (!m_mesh)
		{
			cout << "ERROR: EnergyToFluence::results() requested but no attached mesh" << endl;
			return;
		}
		if (auto p = dynamic_cast<const SpatialMap<DirectedSurfaceElement<float>>*>(m_values))
			C = new DirectedSurfaceConverter(m_mesh);
		else if (auto p = dynamic_cast<const SpatialMap<float>*>(m_values))
			C = new SurfaceConverter(m_mesh);
		break;

	case AbstractSpatialMap::Volume:
		if (!m_mesh)
		{
			cout << "ERROR: EnergyToFluence::results() requested but no attached mesh" << endl;
			return;
		}
		if (!m_materials)
		{
			cout << "ERROR: EnergyToFluence::results() requested but no material information provided" << endl;
			return;
		}
		C = new VolumeConverter(m_mesh,m_materials);
		break;

	default:
		cout << "ERROR: EnergyToFluence::results() requested but input is neither a surface nor a volume map" << endl;
		return;
	}

	if (!C)
		return;

	// Clone input to get same dimensions and type
	m_output = static_cast<AbstractSpatialMap*>(m_values->clone());
	m_output->quantity(&Units::fluence);

	if (auto mf = dynamic_cast<const SpatialMap<float>*>(m_values))
	{
		for(unsigned i=0;i<m_values->dim();++i)
			static_cast<SpatialMap<float>&>(*m_output)[i] = (*C)(i,(*mf)[i]);
	}
	else if (auto md = dynamic_cast<const SpatialMap<double>*>(m_values))
	{
		for(unsigned i=0;i<m_values->dim();++i)
			static_cast<SpatialMap<double>&>(*m_output)[i] = (*C)(i,(*md)[i]);
	}
	else if (auto mdir = dynamic_cast<const SpatialMap<DirectedSurfaceElement<float>>*>(m_values))
	{
		for(unsigned i=0;i<m_values->dim();++i)
			static_cast<SpatialMap<DirectedSurfaceElement<float>>&>(*m_output)[i] = (*C)(i,(*mdir)[i]);
		m_output->quantity(&Units::directedFluence);
	}
	else
		cout << "ERROR: EnergyToFluence::results() requested but input array type is not recognized" << endl;

	delete C;
}

OutputData* EnergyToFluence::result() const
{
	if (m_output)
		return m_output;

	cout << "ERROR! Returning a null pointer from EnergyToFluence::result()" << endl;
	return nullptr;
}

