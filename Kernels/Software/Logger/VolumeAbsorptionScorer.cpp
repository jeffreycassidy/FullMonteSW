/*
 * VolumeAbsorptionScorer.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: jcassidy
 */

#include "VolumeAbsorptionScorer.hpp"

#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <FullMonteSW/OutputTypes/OutputData.hpp>
#include <FullMonteSW/Geometry/Units/BaseUnit.hpp>

VolumeAbsorptionScorer::VolumeAbsorptionScorer()
{
}

VolumeAbsorptionScorer::~VolumeAbsorptionScorer()
{
}

void VolumeAbsorptionScorer::dim(unsigned N)
{
	std::cout << "Resizing VolumeAbsorptionScorer accumulation array to " << N << endl;
	m_acc.resize(N);
	std::cout << " size after: " << m_acc.size() << endl;
}

void VolumeAbsorptionScorer::queueSize(unsigned q)
{
	m_queueSize=q;
}

std::list<OutputData*> VolumeAbsorptionScorer::results() const
{
	// convert to float
	std::vector<float> vf(m_acc.size());

	for(unsigned i=0;i<m_acc.size();++i)
		vf[i] = m_acc[i];

	// create vector
	SpatialMap<float> *vmap = new SpatialMap<float>(std::move(vf),AbstractSpatialMap::Volume,AbstractSpatialMap::Scalar);

	vmap->quantity(&Units::energy);
	vmap->units(&Units::packet);
	return std::list<OutputData*>{vmap};
}

void VolumeAbsorptionScorer::clear()
{
}

VolumeAbsorptionScorer::Logger VolumeAbsorptionScorer::get_logger()
{
	return VolumeAbsorptionScorer::Logger(m_queueSize);
}

VolumeAbsorptionScorer::Logger::Logger(std::size_t qSize) :
	m_handle(qSize)
{

}

void VolumeAbsorptionScorer::Logger::clear()
{
	m_handle.clear();
}

void VolumeAbsorptionScorer::Logger::commit(AbstractScorer& S)
{
 	m_handle.commit(static_cast<Scorer&>(S).m_acc);
}
