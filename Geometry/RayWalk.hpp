/*
 * RayWalk.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: jcassidy
 */

#include <array>

#include "newgeom.hpp"

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>

class TetraMesh;

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

	static RayWalkIterator init(const TetraMesh& M,const std::array<double,3> p0,const std::array<double,3> dir);

	/// boost::iterator_facade requirements
	const WalkSegment& dereference() const;
	bool equal(const RayWalkIterator& rhs) const;
	void increment();

	/// Calculate a setup when inside a tetra
	void stepInTetra();

	/// Calculate a step outside a tetra
	void stepExterior();

private:
	const TetraMesh* 		m_mesh=nullptr;									// the mesh being walked
	std::array<double,3> 	m_dir=std::array<double,3>{NAN,NAN,NAN};		// the direction of walk

	WalkSegment				m_currSeg;										// current segment (dereference value)
	unsigned				nextTet_=0;
};
