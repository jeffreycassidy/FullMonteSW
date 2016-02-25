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
	ImplicitPlane(Kernel::Vector n,Kernel::Scalar c) :
		m_normal(n),
		m_offset(c){}

	ImplicitPlane(Kernel::Vector n,Kernel::Point p) :
		m_normal(n),
		m_offset(dot(p,n)){}

	Kernel::UnitVector	normal()						const 	{ return m_normal;		}
	void				normal(Kernel::UnitVector n)			{ m_normal=n;			}

	Kernel::Scalar		offset()						const	{ return m_offset;		}
	void				offset(Kernel::Scalar c)				{ m_offset=c;			}

	Kernel::Point		origin()						const 	{ return m_normal*m_offset;}


	/// Evaluate implicit function
	Kernel::Scalar operator()(Kernel::Point p) const
	{
		return dot(p,m_normal)-m_offset;
	}

private:
	Kernel::UnitVector	m_normal;
	Kernel::Scalar		m_offset=0;
};



#endif /* GEOMETRY_IMPLICITPLANE_HPP_ */
