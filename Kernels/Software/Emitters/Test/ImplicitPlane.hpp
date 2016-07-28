/*
 * ImplicitPlane.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_IMPLICITPLANE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_IMPLICITPLANE_HPP_

#include <array>
#include <FullMonteSW/Geometry/StandardArrayKernel.hpp>

/** Support code providing an implicit plane (for disk/triangle) */

template<typename FT,std::size_t D>struct ImplicitPlane
{
	ImplicitPlane(){}

	ImplicitPlane(std::array<FT,D> n,FT c) :
		m_n(n),m_c(c){}

	ImplicitPlane(std::array<FT,D> n,std::array<FT,D> p0) :
		m_n(n),m_c(0.0)
	{
		m_c = -operator()(p0);
	}

	/// Evaluates the implicit function (point height over plane)
	FT operator()(std::array<FT,D> p) const
	{
		return dot(p,m_n)-m_c;
	}

	/// Flips the orientation of the plane
	void flip()
	{
		m_n = -m_n;
		m_c=-m_c;
	}

	std::array<FT,D> 	m_n;
	FT					m_c;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_IMPLICITPLANE_HPP_ */
