/*
 * CylAbsorption.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#include "CylAbsorption.hpp"

CylAbsorptionScorer::CylAbsorptionScorer()
{
}

CylAbsorptionScorer::~CylAbsorptionScorer()
{
}

void CylAbsorptionScorer::dim(unsigned Nr,unsigned Nz)
{
	m_Nr = Nr;
	m_Nz = Nz;
	m_acc.resize(Nr*Nz);
}

void CylAbsorptionScorer::extent(float r,float z)
{
	m_dr = r/float(N);
	m_dz = z/float(N);
}

void CylAbsorptionScorer::resolution(float dr,float dz)
{
	m_dr = dr;
	m_dz = dz;
}

std::list<OutputData*> CylAbsorptionScorer::results() const
{
//	// convert to float
//	std::vector<float> vf(m_acc.size());
//
//	for(unsigned i=0;i<m_acc.size();++i)
//		vf[i] = m_acc[i];
//
//	// create vector
//	SpatialMap<float> *vmap = new SpatialMap<float>(std::move(vf),AbstractSpatialMap::Volume,AbstractSpatialMap::Scalar);
//	return std::list<OutputData*>{vmap};
}

void CylAbsorptionScorer::clear()
{
}

CylAbsorptionScorer::Logger CylAbsorptionScorer::get_logger()
{
	return CylAbsorptionScorer::Logger();
}

CylAbsorptionScorer::Logger::Logger()
{
}

void CylAbsorptionScorer::Logger::clear()
{
	m_handle.clear();
}

void CylAbsorptionScorer::Logger::commit(AbstractScorer& S)
{
 	//m_handle.commit(static_cast<Scorer&>(S).m_acc);
}
