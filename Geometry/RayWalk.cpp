/*
 * RayWalk.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#include "TetraMesh.hpp"
#include "RayWalk.hpp"

const WalkSegment& RayWalkIterator::dereference() const
{
	return m_currSeg;
}

/** Equality: either both in same tetra, or both at end */
bool RayWalkIterator::equal(const RayWalkIterator& rhs) const
{
	if (m_mesh == nullptr)				// if this == end, check if rhs == end too
		return rhs.m_mesh==nullptr;
	else							// if this != end, check if in same nonzero tetra (zero tetra is non-unique)
		return m_currSeg.IDt != 0 && m_currSeg.IDt == rhs.m_currSeg.IDt;
}

void RayWalkIterator::increment()
{
	m_currSeg.f0 = m_currSeg.f1;
	m_currSeg.dToOrigin += m_currSeg.lSeg;

	m_currSeg.IDt = nextTet_;
	m_currSeg.matID = m_mesh->getMaterial(nextTet_);

	if (m_currSeg.IDt == 0)		// not currently in a tetra, need to check faces exhaustively
	{
		stepExterior();
		if (m_currSeg.f1.IDf == 0)		// no face found: return end iterator
			*this = RayWalkIterator();
	}
	else				// in a tetra, just four faces to check for next intersection (always will have, since we're in tetra)
		stepInTetra();
}

RayWalkIterator RayWalkIterator::init(const TetraMesh& M,const std::array<double,3> p0,const std::array<double,3> dir)
{
	RayWalkIterator rwi;

	rwi.m_mesh=&M;
	rwi.m_dir=dir;

	rwi.m_currSeg.f0.p 	= rwi.m_currSeg.f1.p = p0;		// origin point
	rwi.m_currSeg.f0.IDf = 0;							// not-a-face

	rwi.m_currSeg.dToOrigin = 0;			// starts at ray origin

	// find the enclosing tetra
	rwi.m_currSeg.IDt = M.findEnclosingTetra(Point<3,double>{ p0[0], p0[1], p0[2]});
	rwi.m_currSeg.matID = M.getMaterial(rwi.m_currSeg.IDt);

	if (rwi.m_currSeg.IDt == 0)			// doesn't start in a tetra; search faces
	{
		rwi.stepExterior();

		if (rwi.m_currSeg.f1.IDf==0)
			return RayWalkIterator();
	}
	else						// in a tetra, so search tetra's faces
		rwi.stepInTetra();


	//std::array<double,3> v01{ rwi.currSeg_.f1.p[0]-p0[0], rwi.currSeg_.f1.p[1]-p0[1],rwi.currSeg_.f1.p[2]-p0[2]} ;

	return rwi;
}


/** Completes the current step within a tetra, starting at intersection defined by currSeg_.f1 in tetra currSeg_.IDt
 * Updates currSeg_.f1, .lSeg, and nextTet_
 */
void RayWalkIterator::stepInTetra()
{
	__m128 p = _mm_setr_ps(m_currSeg.f1.p[0],m_currSeg.f1.p[1],m_currSeg.f1.p[2],0);
	__m128 d = _mm_setr_ps(m_dir[0],m_dir[1],m_dir[2],0);
	__m128 s = _mm_set1_ps(std::numeric_limits<float>::infinity());
	StepResult sr = m_mesh->getTetra(m_currSeg.IDt).getIntersection(p,d,s);

	float t[4];
	_mm_store_ps(t,sr.Pe);

	m_currSeg.f1.p =  Point<3,double>{t[0],t[1],t[2]};
	m_currSeg.f1.IDf= sr.IDfe;

	_mm_store_ss(t,sr.distance);

	m_currSeg.lSeg = t[0];

	if (t[0] == std::numeric_limits<float>::infinity())
		nextTet_ = 0;
	else
		nextTet_ = sr.IDte; // the tetra hit by the ray
}

/** Completes the current step outside of a tetra, either ending in a tetra or no intersection (f1.ID==0)
 * Sets currSeg_.f1, .lSeg, nextTet_
 */

void RayWalkIterator::stepExterior()
{
	int IDf;
	PointIntersectionResult res;

	// find the next face along the ray, excluding the current face from consideration (may see hit due to tolerance errors)
	std::tie(res,IDf) = m_mesh->findNextFaceAlongRay(m_currSeg.f1.p, UnitVector<3,double>{m_dir[0],m_dir[1],m_dir[2]},m_currSeg.f1.IDf);

	m_currSeg.f1.p = res.q;
	m_currSeg.lSeg = res.t;

	unsigned IDt;

	// check if + orientation of this face points into the current tetra. if so, take other tetra and return -face.
	if ((IDt=m_mesh->getTetraFromFace(IDf)) == m_currSeg.IDt)
	{
		IDf=-IDf;
		IDt = m_mesh->getTetraFromFace(IDf);
	}

	m_currSeg.f1.IDf = IDf;
	nextTet_ = IDt;
	m_currSeg.IDt = 0;
	m_currSeg.matID = 0;

	m_currSeg.lSeg=res.t;
}
