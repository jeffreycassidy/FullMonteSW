/*
 * SurfaceExitScorer.cpp
 *
 *  Created on: Sep 29, 2016
 *      Author: jcassidy
 */

#include "SurfaceExitScorer.hpp"

#include <FullMonteSW/OutputTypes/OutputData.hpp>
#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

#include <FullMonteSW/Geometry/Units/BaseUnit.hpp>

#include <cassert>
#include <boost/range/algorithm.hpp>

SurfaceExitScorer::SurfaceExitScorer()
{
}

SurfaceExitScorer::~SurfaceExitScorer()
{
}

void SurfaceExitScorer::dim(std::size_t N)
{
	m_elements=N;
	m_acc.resize(2*N+1);

	std::cout << "Underlying accumulator is of size " << m_acc.size() << endl;
}

std::list<OutputData*> SurfaceExitScorer::results() const
{
	// convert to float
	std::vector<float> vf(m_elements);

	// [-Nf,Nf] + Nf -> [0..2 Nf]

	vf[0] = m_acc[m_elements];

	for(unsigned i=1;i<m_elements;++i)
		vf[i] = m_acc[i+m_elements] + m_acc[m_elements-i];

	// create vector
	SpatialMap<float> *smap = new SpatialMap<float>(std::move(vf),AbstractSpatialMap::Surface,AbstractSpatialMap::Scalar);
	smap->quantity(&Units::energy);
	smap->units(&Units::packet);
	return std::list<OutputData*>{smap};
}

void SurfaceExitScorer::clear()
{
	m_acc.clear();
}

SurfaceExitScorer::Logger SurfaceExitScorer::get_logger()
{
	return Logger();
}

double SurfaceExitScorer::total() const
{
	double Esum=0.0;
	for(unsigned i=0;i<m_acc.size();++i)
		Esum += m_acc[i];
	cout << "INFO: Summed " << m_acc.size() << " elements to get a total of " << Esum << endl;
	return Esum;
}

SurfaceExitScorer::Logger::Logger()
{
}

SurfaceExitScorer::Logger::~Logger()
{
}
