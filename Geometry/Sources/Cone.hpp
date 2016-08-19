/*
 * Cone.hpp
 *
 *  Created on: Aug 18, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_CONE_HPP_
#define GEOMETRY_SOURCES_CONE_HPP_

/** Defines a conical beam emerging from a fiber source of a given numerical aperture.
 *
 */

class Cone
{
public:
	Cone();
	~Cone();

	void numericalAperture(float na);
	void halfAngle(float theta);
	void refractiveIndex(float n);

	float numericalAperture() const;
	float halfAngle() const;
	float refractiveIndex() const;

private:
	float m_halfAngle=0.0f;
};




#endif /* GEOMETRY_SOURCES_CONE_HPP_ */
