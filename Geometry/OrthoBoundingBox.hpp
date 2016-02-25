/*
 * OrthoBoundingBox.hpp
 *
 *  Created on: Jan 28, 2015
 *      Author: jcassidy
 */

#ifndef GEOMETRY_ORTHOBOUNDINGBOX_HPP_
#define GEOMETRY_ORTHOBOUNDINGBOX_HPP_

#include <array>
#include <utility>

#include <limits>
#include <functional>

template<class Kernel>class OrthoBoundingBox {
public:
	OrthoBoundingBox();
	OrthoBoundingBox(Kernel::Point,Kernel::Point);

	OrthoBoundingBox(OrthoBoundingBox&&) = delete;
	OrthoBoundingBox(OrthoBoundingBox)   = default;

	/// Query whether a point is within the box
	bool contains(const Kernel::Point p) const;

	/// Update box to contains point
	void insert(const Kernel::Point p);

	// Query box corners (min/max)
	std::pair<Kernel::Point,Kernel::Point> corners() const;

	/// Query dimensions of the box
	Kernel::Vector dims() const;

	/** Calculate an isotropic scale that maps the bounding box so that it is centered in and fills (fillfactor%) of a box of size
	 * lims
	 *
	 * design scale and offset such that:
	 *		mincorner  	<-> (lims[]-k*dims[])/2
	 *		maxcorner  	<-> (lims[]+k*dims[])/2
	 *		center 		<-> lims[]/2
	 */
	std::pair<Kernel::FT,Kernel::Vector> scale_offset(const Kernel::Vector lims,Kernel::FT fillfactor=FT(0.95f)) const;

private:
	std::pair<Kernel::Point,Kernel::Point> m_corners;
};

template<class Kernel>OrthoBoundingBox<Kernel>::OrthoBoundingBox()
{

}


//	boost::fill(_bbmin,std::numeric_limits<T>::max());
//	boost::fill(_bbmax,std::numeric_limits<T>::lowest());


template<class Kernel>void OrthoBoundingBox<Kernel>::insert(Kernel::Point p)
{
	m_corners = std::make_pair(elementwise_min(p,m_corners.first),elementwise_max(p,m_corners.second));
}

template<class Kernel>bool OrthoBoundingBox<Kernel>::query(Kernel::Point p) const
{
	return !(elementwise_gt(p,m_corners.second) | elementwise_lt(p,m_corners.first));
}


{ return _bbmax-_bbmin; }
{ return std::make_pair(_bbmin,_bbmax);}

std::pair<double,std::array<T,D>> scale_offset(const std::array<T,D> lims,double fillfactor=0.95) const
{
	// derive scale
	std::array<T,D> d = dims();
	double k = lims[0]/d[0];
	for(unsigned i=1;i<D;++i)
		k = min(k,lims[i]/d[i]);
	k *= fillfactor;

	// derive offset from scale
	std::array<T,D> p0;
	for(unsigned i=0;i<D;++i)
		p0[i] = (lims[i]-k*d[i])/2-k*_bbmin[i];

	return std::make_pair(k,p0);
}
#endif /* BOUNDINGBOX_HPP_ */
