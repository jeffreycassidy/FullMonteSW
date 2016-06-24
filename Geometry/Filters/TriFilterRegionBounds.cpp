/*
 * TriFilterRegionBounds.cpp
 *
 *  Created on: Mar 16, 2016
 *      Author: jcassidy
 */

#include "TriFilterRegionBounds.hpp"

#include <FullMonteSW/Geometry/TetraMesh.hpp>

bool TriFilterRegionBounds::operator()(int IDf) const
{
	unsigned IDtA = mesh()->getTetraFromFace(IDf);			// tetra coming from
	unsigned IDtB = mesh()->getTetraFromFace(-IDf);			// tetra going to

	unsigned mA = mesh()->getMaterial(IDtA);
	unsigned mB = mesh()->getMaterial(IDtB);

	return (mB != mA
			&& ((mA < m_regions.size() && m_regions[mA])
					|| (m_bidirectional && mB < m_regions.size() && m_regions[mB])
				)
			);
}

bool TriFilterRegionBounds::bidirectional() const
{
	return m_bidirectional;
}

void TriFilterRegionBounds::bidirectional(bool bidir)
{
	m_bidirectional=bidir;
}

void TriFilterRegionBounds::includeRegion(unsigned r,bool incl)
{
	if (r >= m_regions.size())
		m_regions.resize(r+1,false);
	m_regions[r] = incl;
}
