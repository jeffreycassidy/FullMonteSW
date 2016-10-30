/*
 * InternalSurface.hpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_INTERNALSURFACE_HPP_
#define OUTPUTTYPES_INTERNALSURFACE_HPP_

template<typename T>class DirectedSurfaceElement
{
public:
	DirectedSurfaceElement(){}
	DirectedSurfaceElement(T exit_,T enter_) : m_enter(enter_),m_exit(exit_){}

	T enter() 			const { return m_enter; 	}
	T exit()			const { return m_exit; 	}
	T bidirectional() 	const { return m_exit+m_enter; }

	DirectedSurfaceElement& operator+=(const DirectedSurfaceElement rhs)
	{
		m_exit += rhs.m_exit;
		m_enter += rhs.m_enter;
		return *this;
	}

	void flip()
	{
		std::swap(m_enter,m_exit);
	}

private:
	T m_enter=0;
	T m_exit=0;
};

#endif /* OUTPUTTYPES_INTERNALSURFACE_HPP_ */
