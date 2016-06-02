/*
 * ImplicitPlane.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_IMPLICITPLANE_HPP_
#define GEOMETRY_IMPLICITPLANE_HPP_

template<class Kernel>class ImplicitPlane
{
public:
	explicit ImplicitPlane(typename Kernel::Vector n=Kernel::zeroVector(),typename Kernel::Scalar c=0) :
		m_normal(n),
		m_offset(c){}

	ImplicitPlane(typename Kernel::Vector n,typename Kernel::Point p) :
		m_normal(n),
		m_offset(dot(p,n)){}

	typename Kernel::UnitVector	normal()								const 	{ return m_normal;		}
	void						normal(typename Kernel::UnitVector n)			{ m_normal=n;			}

	typename Kernel::Scalar		offset()								const	{ return m_offset;		}
	void						offset(typename Kernel::Scalar c)				{ m_offset=c;			}

	typename Kernel::Point		origin()								const 	{ return m_normal*m_offset; }


	/// Evaluate implicit function
	typename Kernel::Scalar operator()(typename Kernel::Point p) const
	{
		return Kernel::dot(p,m_normal)-m_offset;
	}

private:
	typename Kernel::UnitVector	m_normal;
	typename Kernel::Scalar		m_offset=0;
};



#endif /* GEOMETRY_IMPLICITPLANE_HPP_ */
