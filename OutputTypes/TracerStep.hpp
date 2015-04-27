/*
 * TracerStep.hpp
 *
 *  Created on: Apr 21, 2015
 *      Author: jcassidy
 */

#ifndef TRACERSTEP_HPP_
#define TRACERSTEP_HPP_

#include <array>

struct TracerStep {
	enum EventType { Launch, Absorb, Boundary, ReflectFresnel, ReflectInternal, Refract, Scatter, Exit };

	EventType event;
	float weight;
	std::array<float,3> pos;
	float t;
};



#endif /* TRACERSTEP_HPP_ */
