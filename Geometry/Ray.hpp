/*
 * Ray.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_RAY_HPP_
#define GEOMETRY_RAY_HPP_

template<class Kernel>class Ray
{
public:
	Ray(Kernel::Point p0,Kernel::UnitVector direction);

	Kernel::Point		origin()						const 	{ return m_origin;		}
	void				origin(Kernel::Point p)					{ m_origin=p;			}

	void 				direction(Kernel::UnitVector d)			{ m_direction=d;		}
	Kernel::UnitVector 	direction()						const	{ return m_direction;	}

	Kernel::Point		operator()(Kernel::Scalar x)	const
	{
		return m_origin + x*m_direction;
	}

private:
	Kernel::Point		m_origin;
	Kernel::UnitVector	m_direction;
};

template<class Kernel>Ray<Kernel>::Ray(Kernel::Point p0,Kernel::UnitVector direction)
	m_origin(p0),
	m_direction(direction){}
{
}







#endif /* GEOMETRY_RAY_HPP_ */
