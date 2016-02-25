/*
 * TetraFilterByRegion.cpp
 *
 *  Created on: Feb 25, 2016
 *      Author: jcassidy
 */

#include "TetraFilterByRegion.hpp"
#include <boost/range/algorithm.hpp>

#include <FullMonte/Geometry/TetraMeshBase.hpp>

TetraFilterByRegion::TetraFilterByRegion(const TetraMeshBase* m) :
	TetraMeshBaseFilter(m)
{
	postMeshUpdate();
}

void TetraFilterByRegion::include(unsigned region,bool incl)
{
	m_includeFlags.at(region) = incl;
}

bool TetraFilterByRegion::include(unsigned region) const
{
	return m_includeFlags.at(region);
}

void TetraFilterByRegion::includeAll()
{
	boost::fill(m_includeFlags,true);
}

void TetraFilterByRegion::excludeAll()
{
	boost::fill(m_includeFlags,false);
}

void TetraFilterByRegion::postMeshUpdate()
{
	std::cout << "Mesh tetras: " << mesh()->getNt()+1 << " regions: " << mesh()->getNumberOfRegions() << std::endl;
	m_includeFlags.resize(mesh() ? mesh()->getNumberOfRegions()+1 : 0,true);
}

bool TetraFilterByRegion::operator()(const unsigned tet) const
{
	return include(mesh()->getMaterial(tet));
}

