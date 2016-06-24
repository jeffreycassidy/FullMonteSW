/*
 * Face.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TRIANGLE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TRIANGLE_HPP_

namespace Emitter
{

/** Uniform distribution over the surface of a triangle */

template<class RNG>class Triangle
{
public:

	Triangle(){}
	Triangle(unsigned IDt,SSE::Vector3 A,SSE::Vector3 B,SSE::Vector3 C);

	std::pair<unsigned,SSE::Point3> position(RNG& rng) const;

private:
	SSE::Vector3 m_origin;
	SSE::Vector3 m_vectorA;
	SSE::Vector3 m_vectorB;

	unsigned m_IDt=-1U;
};

template<class RNG>Triangle<RNG>::Triangle(unsigned IDt,SSE::Vector3 A,SSE::Vector3 B,SSE::Vector3 C) :
		m_IDt(IDt)
{
	m_origin = A;
	m_vectorA = B-A;
	m_vectorB = C-A;
};

template<class RNG>std::pair<unsigned,SSE::Point3> Triangle<RNG>::position(RNG& rng) const
{
    float s=*rng.floatU01(),t=*rng.floatU01();

    // TODO: SSEify? Fold into RNG to parallelize?

    if (s+t > 1)
        s=1-s,t=1-t;

    return std::make_pair(m_IDt,m_origin + m_vectorA*SSE::Scalar(s) + m_vectorB*SSE::Scalar(t));
}

};

#endif /* KERNELS_SOFTWARE_EMITTERS_TRIANGLE_HPP_ */
