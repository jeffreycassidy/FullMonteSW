/*
 * RayWalk.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: jcassidy
 */

#include "TetraMesh.hpp"
#include "Geometry.hpp"

#include <FullMonte/Storage/TIMOS/TIMOS.hpp>


#include <array>

#include <boost/range/adaptor/indexed.hpp>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>

struct WalkFaceInfo
{
	Point<3,double> p={NAN,NAN,NAN};	// the point of intersection
	int IDf=0;							// face at which intersection happens (0 = no-face)
};

struct WalkSegment {
	WalkFaceInfo f0, f1;		// starting/ending face & point info

	double lSeg=0;				// length of the segment
	double dToOrigin=0;			// distance from segment start to origin

	unsigned IDt=0;				// current tetra (=0 if exterior to mesh)
	unsigned matID=0;			// current material (=0 if exterior to mesh)
};



/** Iterator to "walk" along a ray, providing information about each segment (start/end face & point, tet/mat ID, length).
 *
 *
 * begin 	is always at the start point, whether or not contained by a tetra (IDt=matID=0 if outside, else enclosing tetra)
 *
 * end		default constructor value (no mesh/direction/point, IDt=IDf=0)
 * 			is a "null-like" value indicating no further intersections
 *
 * The walk may pass outside of the mesh (tet 0), but it will re-enter if the ray passes through a nonzero material again.
 * The last step reported ends at the final boundary with region 0 (the infinite continuation is not included)
 *
 * Consequently, if the ray never passes through material then begin()==end()
 */



class RayWalkIterator : public boost::iterator_facade<
	RayWalkIterator,
	const WalkSegment,
	std::forward_iterator_tag,
	const WalkSegment&,
	std::ptrdiff_t>
{

public:

	// default initializer yields end() for all walks
	RayWalkIterator(){}
	RayWalkIterator(const RayWalkIterator&) = default;

	const WalkSegment& dereference() const { return currSeg_; }

	/** Equality only works for */

	bool equal(const RayWalkIterator& rhs) const
	{
		if (M_ == nullptr)				// if this == end, check if rhs == end too
			return rhs.M_==nullptr;
		else							// if this != end, check if in same nonzero tetra (zero tetra is non-unique)
			return currSeg_.IDt != 0 && currSeg_.IDt == rhs.currSeg_.IDt;
	}

	// move to the next intersection
	void increment()
	{
		currSeg_.f0 = currSeg_.f1;
		currSeg_.dToOrigin += currSeg_.lSeg;

		currSeg_.IDt = nextTet_;
		currSeg_.matID = M_->getMaterial(nextTet_);

		if (currSeg_.IDt == 0)		// not currently in a tetra, need to check faces exhaustively
		{
			stepExterior();
			if (currSeg_.f1.IDf == 0)		// no face found: return end iterator
				*this = RayWalkIterator();
		}
		else				// in a tetra, just four faces to check for next intersection (always will have, since we're in tetra)
			stepInTetra();
	}

	static RayWalkIterator init(const TetraMesh& M,const std::array<double,3> p0,const std::array<double,3> dir)
	{
		RayWalkIterator rwi;

		rwi.M_=&M;
		rwi.dir_=dir;

		rwi.currSeg_.f0.p 	= rwi.currSeg_.f1.p = p0;		// origin point
		rwi.currSeg_.f0.IDf = 0;							// not-a-face

		rwi.currSeg_.dToOrigin = 0;			// starts at ray origin

		// find the enclosing tetra
		rwi.currSeg_.IDt = M.findEnclosingTetra(Point<3,double>{ p0[0], p0[1], p0[2]});
		rwi.currSeg_.matID = M.getMaterial(rwi.currSeg_.IDt);

		if (rwi.currSeg_.IDt == 0)			// doesn't start in a tetra; search faces
		{
			rwi.stepExterior();

			if (rwi.currSeg_.f1.IDf==0)
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
	void stepInTetra()
	{
		__m128 p = _mm_setr_ps(currSeg_.f1.p[0],currSeg_.f1.p[1],currSeg_.f1.p[2],0);
		__m128 d = _mm_setr_ps(dir_[0],dir_[1],dir_[2],0);
		__m128 s = _mm_set1_ps(std::numeric_limits<float>::infinity());
		StepResult sr = M_->getTetra(currSeg_.IDt).getIntersection(p,d,s);

		float t[4];
		_mm_store_ps(t,sr.Pe);

		currSeg_.f1.p =  Point<3,double>{t[0],t[1],t[2]};
		currSeg_.f1.IDf= sr.IDfe;

		_mm_store_ss(t,sr.distance);

		currSeg_.lSeg = t[0];

		nextTet_ = sr.IDte; // the tetra hit by the ray
	}

	/** Completes the current step outside of a tetra, either ending in a tetra or no intersection (f1.ID==0)
	 * Sets currSeg_.f1, .lSeg, nextTet_
	 */

	void stepExterior()
	{
		int IDf;
		PointIntersectionResult res;

		// find the next face along the ray, excluding the current face from consideration (may see hit due to tolerance errors)
		std::tie(res,IDf) = M_->findNextFaceAlongRay(currSeg_.f1.p, UnitVector<3,double>{dir_[0],dir_[1],dir_[2]},currSeg_.f1.IDf);

		currSeg_.f1.p = res.q;
		currSeg_.lSeg = res.t;

		unsigned IDt;

		// check if + orientation of this face points into the current tetra. if so, take other tetra and return -face.
		if ((IDt=M_->getTetraFromFace(IDf)) == currSeg_.IDt)
		{
			IDf=-IDf;
			IDt = M_->getTetraFromFace(IDf);
		}

		currSeg_.f1.IDf = IDf;
		nextTet_ = IDt;
		currSeg_.IDt = 0;

		currSeg_.lSeg=res.t;
	}

	// void decrement()
	// void advance()
	// std::ptrdiff_t distance_to()

private:
	const TetraMesh* 		M_=nullptr;				// the mesh being walked
	std::array<double,3> 	dir_=std::array<double,3>{NAN,NAN,NAN};		// the direction of walk

	WalkSegment				currSeg_;				// current segment (dereference value)
	unsigned				nextTet_=0;
};



