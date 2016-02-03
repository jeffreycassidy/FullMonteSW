/*
 * Tetra.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TETRA_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TETRA_HPP_

#include "../SSEMath.hpp"

#include <cassert>

namespace Emitter
{

/** Uniform distribution over the volume of a tetrahedron */

template<class RNG>class Tetra
{
public:
	/// Defines the tetra by the four vertices
	Tetra(SSE::Vector3 A,SSE::Vector3 B,SSE::Vector3 C,SSE::Vector3 D);

	/// Calculates the position given random numbers from the generator
	SSE::Vector3 position(RNG& rng) const;

private:
	std::array<SSE::Vector3,3>	m_matrix;				///< Matrix to shear unit tetra into direction
	SSE::Vector3				m_origin;				///< Offset (point A of tetra)
};


template<class RNG>Tetra<RNG>::Tetra(SSE::Vector3 A,SSE::Vector3 B,SSE::Vector3 C,SSE::Vector3 D)
{
	// copy first point
	m_origin = A;

	SSE::Vector3 AB=B-A;
	SSE::Vector3 AC=C-A;
	SSE::Vector3 AD=D-A;

	m_matrix[0] = AB;
	m_matrix[1] = AC;
	m_matrix[2] = AD;

	m_origin = A;
}

template<class RNG>SSE::Vector3 Tetra<RNG>::position(RNG& rng) const
{
    std::array<float,3> p;

    // TODO: SSEify the items below? Fold into RNG to parallelize?

    std::array<float,3> rnd{ rng.draw_float_u01(), rng.draw_float_u01(), rng.draw_float_u01() };
    if (rnd[0]+rnd[1] > 1.0)
    {
        rnd[0] = 1.0-rnd[0];
        rnd[1] = 1.0-rnd[1];
    }

    if (rnd[0]+rnd[1]+rnd[2] > 1.0)
    {
        if(rnd[1]+rnd[2] < 1.0)
        {
            p[0] = 1-rnd[1]-rnd[2];
            p[1] = rnd[1];
            p[2] = rnd[0]+rnd[1]+rnd[2] - 1;
        }
        else {
            p[0] = rnd[0];
            p[1] = 1-rnd[2];
            p[2] = 1-rnd[0]-rnd[1];
        }
    }
    else {
        p[0]=rnd[0];
        p[1]=rnd[1];
        p[2]=rnd[2];
    }

    assert(p[0] >= 0.0 && p[1] >= 0.0 && p[2] >= 0.0 && p[0]+p[1]+p[2] <= 1.0);

    // TODO: Better matrix multiply code?
    return m_origin + m_matrix[0]*SSE::Scalar(p[0]) + m_matrix[1]*SSE::Scalar(p[1]) + m_matrix[2]*SSE::Scalar(p[2]);
}


};


#endif /* KERNELS_SOFTWARE_EMITTERS_TETRA_HPP_ */
