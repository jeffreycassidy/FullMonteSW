/*
 * FluenceConverter.hpp
 *
 *  Created on: Feb 20, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_FLUENCECONVERTER_HPP_
#define OUTPUTTYPES_FLUENCECONVERTER_HPP_

class TetraMesh;

#include <FullMonteSW/Geometry/SimpleMaterial.hpp>

class EnergyToFluence
{
public:
	EnergyToFluence();
	~EnergyToFluence();

	const TetraMesh* 					mesh() 											const 	{ return m_mesh; 		}
	void 								mesh(const TetraMesh* m)								{ m_mesh=m;				}

	const std::vector<SimpleMaterial>*	materials()										const	{ return m_materials;	}
	void 								materials(const std::vector<SimpleMaterial>* m)			{ m_materials=m;		}

	void								source(OutputData* M);
	const AbstractSpatialMap*			source() const;

	void 								update();

	OutputData*							result() const;


private:
	class Converter;

	class VolumeConverter;
	class SurfaceConverter;

	const TetraMesh*					m_mesh=nullptr;
	const std::vector<SimpleMaterial>*	m_materials=nullptr;

	const AbstractSpatialMap*			m_values=nullptr;
	AbstractSpatialMap*					m_output=nullptr;
};

#endif /* OUTPUTTYPES_FLUENCECONVERTER_HPP_ */

