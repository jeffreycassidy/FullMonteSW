/*
 * RayWalk.cpp
 *
 *  Created on: Oct 21, 2015
 *      Author: jcassidy
 */

#ifndef GEOMETRY_RAYWALK_HPP_
#define GEOMETRY_RAYWALK_HPP_
#include <array>

#include <boost/iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <boost/range/any_range.hpp>

class TetraMesh;

struct WalkFaceInfo
{
	std::array<float,3> p{{NAN,NAN,NAN}};	// the point of intersection
	int IDf=0;								// face at which intersection happens (0 = no-face) - points into current walk segment
											// ie. points on current walk segment have positive height over face
};

struct WalkSegment {
	WalkFaceInfo f0, f1;		// starting/ending face & point info

	float lSeg=0;				// length of the segment
	float dToOrigin=0;			// distance from segment start to origin

	unsigned IDt=0;				// current tetra (=0 if exterior to mesh)
	unsigned matID=0;			// current material (=0 if exterior to mesh)
};


class LineQuery
{
public:
	LineQuery();

	/// Get/set associated mesh
	const TetraMesh*		mesh() 							const;
	void					mesh(const TetraMesh*);

	/// Get/set origin
	std::array<float,3>		origin() 						const;
	void					origin(std::array<float,3> p);

	/// Get/set direction of travel
	std::array<float,3>		direction()						const;
	void					direction(std::array<float,3> d);

	/// If true, advance until material and tetra ID are both nonzero before returning results (counter starts at 0 where enters mesh)
	bool					skipInitialZeros()				const;
	void					skipInitialZeros(bool skip);

	/// Set direction and length
	void 					destination(std::array<float,3> p1);

	/// Get/set length of segment (preserves origin & direction, extends/truncates from destination)
	float 					length() const;
	void					length(float l);

	typedef boost::any_range<
			const WalkSegment,
			boost::forward_traversal_tag,
			const WalkSegment&,
			std::ptrdiff_t
			> const_range;

	typedef boost::range_iterator<const_range> const_iterator;

	const_range result() const;

private:
	const TetraMesh* 		m_mesh=nullptr;
	std::array<float,3>		m_origin;
	std::array<float,3>		m_direction;
	float					m_length=std::numeric_limits<float>::infinity();

	bool					m_skipInitialZeros=true;
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

	/// Initializes
	static RayWalkIterator init(const TetraMesh& M,const std::array<float,3> p0,const std::array<float,3> dir,float dist=0.0f);

	/// Creates an end iterator at the specified distance from the origin point
	static RayWalkIterator endAt(float d=std::numeric_limits<float>::infinity());

	/// Copies the iterator, but resets the distance to origin to zero
	static RayWalkIterator restartFrom(const RayWalkIterator&);

	/// boost::iterator_facade requirements
	const WalkSegment& dereference() const;
	bool equal(const RayWalkIterator& rhs) const;
	void increment();


private:
	/// Calculate a setup when inside a tetra
	void finishStepInTetra();

	/// Calculate a step outside a tetra
	void finishStepExterior();

	const TetraMesh* 		m_mesh=nullptr;									// the mesh being walked
	std::array<float,3> 	m_dir=std::array<float,3>{{NAN,NAN,NAN}};		// the direction of walk

	WalkSegment				m_currSeg;										// current segment (dereference value)
	unsigned				nextTet_=0;
};
#endif
