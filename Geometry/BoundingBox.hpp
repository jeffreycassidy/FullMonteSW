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

/* Axis-orthogonal bounding box.
 */

template<typename T,size_t D>class OrthoBoundingBox {
public:

	OrthoBoundingBox()
	{
		boost::fill(m_min,std::numeric_limits<T>::max());
		boost::fill(m_max,std::numeric_limits<T>::lowest());
	}

	OrthoBoundingBox(const std::array<T,D>& bbmin_,const std::array<T,D>& bbmax_) : m_min(bbmin_),m_max(bbmax_){}

	/// Returns true if point is inside the closed bounding region
	bool point_inside(const std::array<T,D>& a) const
	{
		for(unsigned i=0;i<D;++i)
			if (a[i] <= m_min[i] || m_max[i] <= a[i])
				return false;
		return true;
	}

	std::function<bool(const std::array<T,D>)> inside_tester() const
	{
		OrthoBoundingBox bbcopy=*this;
		return [bbcopy](const std::array<T,D>& a){ return bbcopy->point_inside(a); };
	}

	/// Insert a point into the bounding box
	void insert(const std::array<T,D>& a)
	{
		for(unsigned i=0;i<D;++i)
		{
			m_min[i] = std::min(m_min[i],a[i]);
			m_max[i] = std::max(m_max[i],a[i]);
		}
	}

	/// Return the dimensions
	std::array<T,D> dims() const
	{
		std::array<T,D> o;
		for(unsigned i=0;i<D;++i)
			o[i] = m_max[i] < m_min[i] ? 0 : m_max[i]-m_min[i];
		return o;
	}

	// calculate an isotropic scale that maps the bounding box so that it is centered in and fills (fillfactor%) of a box of size
	// dims
	//
	// design scale and offset such that:
	//		m_min  <-> (lims[]-k*dims[])/2
	//		m_max  <-> (lims[]+k*dims[])/2
	//		center <-> lims[]/2
	//

	std::pair<double,std::array<T,D>> scale_offset(const std::array<T,D>& lims,double fillfactor=0.95) const
	{
		// derive scale
		std::array<T,D> d = dims();
		double k = lims[0]/d[0];
		for(unsigned i=1;i<D;++i)
			k = std::min(k,lims[i]/d[i]);
		k *= fillfactor;

		// derive offset from scale
		std::array<T,D> p0;
		for(unsigned i=0;i<D;++i)
			p0[i] = (lims[i]-k*d[i])/2-k*m_min[i];

		return std::make_pair(k,p0);
	}

	std::pair<std::array<T,D>,std::array<T,D>> corners() const { return std::make_pair(m_min,m_max);}

private:
	std::array<T,D> m_min;
	std::array<T,D> m_max;
};







#endif /* GEOMETRY_BOUNDINGBOX_HPP_ */
