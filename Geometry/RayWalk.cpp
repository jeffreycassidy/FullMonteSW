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
	Point<3,double> p={NAN,NAN,NAN};
	int IDf=0;
};

struct WalkSegment {
	WalkFaceInfo f0, f1;									// starting/ending face & point info

	double lSeg=std::numeric_limits<double>::infinity();	// length of the segment
	double dToOrigin=0;										// distance from segment start to origin

	unsigned IDt=0;
	unsigned matID=0;
};



/** Iterator to "walk" along a ray, providing information about each segment.
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

	bool equal(const RayWalkIterator& rhs) const
	{
		if (M_ == nullptr)				// if this == end, check if rhs == end too
			return rhs.M_==nullptr;
		else							// if this != end, check if in same nonzero tetra
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
		else				// in a tetra, just four faces to check for next intersection
			stepInTetra();
	}

	static RayWalkIterator init(const TetraMesh& M,const std::array<double,3> p0,const std::array<double,3> dir)
	{
		RayWalkIterator rwi;

		rwi.M_=&M;
		rwi.dir_=dir;

		rwi.currSeg_.f0.p 	= rwi.currSeg_.f1.p = p0;			// origin point
		rwi.currSeg_.f0.IDf = 0;			// not-a-face

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
		{
			rwi.stepInTetra();
		}

		std::array<double,3> v01{ rwi.currSeg_.f1.p[0]-p0[0], rwi.currSeg_.f1.p[1]-p0[1],rwi.currSeg_.f1.p[2]-p0[2]} ;


		return rwi;
	}

	/** Completes the current step within a tetra
	 * Sets f1 & nextTet_
	 */

	void stepInTetra()
	{
		cout << "Stepping within tetra " << currSeg_.IDt << endl;
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

	/** Completes the current step outside of a tetra, either ending in a tetra or no intersection (f1.IDf=0)
	 * Sets f1 & nextTet_
	 */

	void stepExterior()
	{
		cout << "Starting exterior step: ";
		int IDf;
		PointIntersectionResult res;
		std::tie(res,IDf) = M_->findNextFaceAlongRay(currSeg_.f1.p, UnitVector<3,double>{dir_[0],dir_[1],dir_[2]},currSeg_.f1.IDf);

		currSeg_.f1.p = res.q;
		currSeg_.lSeg = res.t;

		unsigned IDt;

		// check if + orientation of this face points into the current tetra. if so, take other tetra and return - face.
		if ((IDt=M_->getTetraFromFace(IDf)) == currSeg_.IDt)
		{
			IDf=-IDf;
			IDt = M_->getTetraFromFace(IDf);
		}

		cout << "Arrived in tetra " << IDt << " via face " << IDf << " at point " << res.q << endl;

		currSeg_.f1.IDf = IDf;
		nextTet_ = IDt;
		currSeg_.IDt = 0;
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

/** Digimouse test case
 * (2.4, 18.8, 0.8) -> (29,1, 13.2, 18.5)
 */

using namespace std;

int main(int argc,char **argv)
{
	// Load up the digimouse
	TIMOSReader R;
	R.setMeshFileName("/Users/jcassidy/src/FullMonteSW/data/mouse.mesh");

	TetraMesh M = R.mesh();

	// Digimouse test case (passes through paw, air, tissue, skull, brain)
	array<double,3> p0{2.4,18.8,0.8}, p1{29.1,13.2,18.5};
	array<double,3> d{0.8214, -0.17220, 0.54428 };		// normalized p1-p0

	RayWalkIterator begin = RayWalkIterator::init(M,p0,d);
	RayWalkIterator end;

	boost::iterator_range<RayWalkIterator> walk(begin,end);

	for(const auto seg : walk | boost::adaptors::indexed(0U))
	{
		cout << "  seg[" << setw(4) << seg.index() << "] in tetra " << seg.value().IDt << " material " << seg.value().matID <<
				" length " << seg.value().lSeg << " distance to origin " << seg.value().dToOrigin << endl;
		cout << "             Starts at face " << seg.value().f0.IDf << " point " << seg.value().f0.p << " runs to face " <<
				seg.value().f1.IDf << " point " << seg.value(). f1.p << endl;
	}

	return 0;

}
