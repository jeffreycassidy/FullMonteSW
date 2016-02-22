/*
 * BoundingBox.hpp
 *
 *  Created on: Jan 28, 2015
 *      Author: jcassidy
 */

#ifndef GEOMETRY_BOUNDINGBOX_HPP_
#define GEOMETRY_BOUNDINGBOX_HPP_

#include <array>
#include <utility>

#include <limits>
#include <functional>


#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>


/* BoundedRegion<T,D> r
 *
 * Valid expressions
 * 		BoundedRegion()					// default empty box
 * 		r.insideTester()				// function object accepting std::array<T,D> and returning bool if inside
 * 		r.inserter()					// function object accepting std::array<T,D> and inserting into the region
 *
 * 		r.corners()						// returns axis-orthogonal box corners
 * 		r.dims()						// returns the size of the axis-orthogonal box
 *
 * 		r.insert(std::array<T,D> p)		// inserts a point
 *
 */


/* OrthoBoundingBox
 *
 * Axis-orthogonal bounding box
 */

template<typename T,size_t D>class OrthoBoundingBox {
public:

	OrthoBoundingBox()
	{
		boost::fill(bbmin_,std::numeric_limits<T>::max());
		boost::fill(bbmax_,std::numeric_limits<T>::lowest());
	}

	OrthoBoundingBox(const std::array<T,D>& bbmin_,const std::array<T,D>& bbmax_) : bbmin_(bbmin_),bbmax_(bbmax_){}

	bool point_inside(const std::array<T,D>& a) const
	{
		for(unsigned i=0;i<D;++i)
			if (a[i] < bbmin_[i] || bbmax_[i] < a[i])
				return false;
		return true;
	}

	std::function<bool(const std::array<T,D>)> inside_tester() const
	{
		OrthoBoundingBox bbcopy=*this;
		return [bbcopy](const std::array<T,D>& a){ return bbcopy->point_inside(a); };
	}

	void insert(const std::array<T,D>& a)
	{
		for(unsigned i=0;i<D;++i)
		{
			bbmin_[i] = std::min(bbmin_[i],a[i]);
			bbmax_[i] = std::max(bbmax_[i],a[i]);
		}

	}

	std::array<T,D> dims() const
	{
		std::array<T,D> o;
		for(unsigned i=0;i<D;++i)
			o[i] = bbmax_[i]-bbmin_[i];
		return o;
	}

	// calculate an isotropic scale that maps the bounding box so that it is centered in and fills (fillfactor%) of a box of size
	// dims
	//
	// design scale and offset such that:
	//		bbmin_  <-> (lims[]-k*dims[])/2
	//		bbmax_  <-> (lims[]+k*dims[])/2
	//		center <-> lims[]/2
	//

	std::pair<double,std::array<T,D>> scale_offset(const std::array<T,D>& lims,double fillfactor=0.95) const
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
			p0[i] = (lims[i]-k*d[i])/2-k*bbmin_[i];

		return std::make_pair(k,p0);
	}

	std::pair<std::array<T,D>,std::array<T,D>> corners() const { return std::make_pair(bbmin_,bbmax_);}

private:

	std::array<T,D> bbmin_;
	std::array<T,D> bbmax_;
};







#endif /* GEOMETRY_BOUNDINGBOX_HPP_ */
