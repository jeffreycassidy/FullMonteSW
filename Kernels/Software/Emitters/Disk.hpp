/*
 * Disk.hpp
 *
 *  Created on: Aug 19, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_DISK_HPP_
#define KERNELS_SOFTWARE_EMITTERS_DISK_HPP_

/** Gives a packet position randomly distributed within a disk of specified centre, radius, and orientation.
 *
 */

namespace Emitter {

template<class RNG>class Disk
{
public:

	Disk();
	Disk(std::array<float,3> centre,std::array<float,3> normal,float radius);

	~Disk();

	std::array<float,3> position(RNG& rng);

private:
	SSE::Point3 						m_centre;
	std::array<SSE::UnitVector3,2> 		m_normalVectors;
	float								m_radius;
};

template<class RNG>Disk<RNG>::Disk()
{

}

template<class RNG>Disk<RNG>::~Disk()
{
}

template<class RNG>Disk<RNG>::Disk(std::array<float,3> centre,std::array<float,3> normal,float radius) :
		m_centre(centre),
		m_radius(radius)
{
	std::tie(m_normalVectors[0],m_normalVectors[1]) = SSE::normalsTo(SSE::UnitVector3(normal));
}

template<class RNG>std::array<float,3> Disk<RNG>::position(RNG& rng)
{
	float rnd0 = *rng.floatU01();
	SSE::UnitVector2 uv = SSE::UnitVector2(_mm_loadu_ps(rng.uvect2D()));

	float r = std::sqrt(rnd0)*m_radius;

	return (m_centre + (m_normalVectors[0]*SSE::Scalar(uv[0]) + m_normalVectors[1]*SSE::Scalar(uv[1]))*SSE::Scalar(r)).array();
}
};

#endif /* KERNELS_SOFTWARE_EMITTERS_DISK_HPP_ */
