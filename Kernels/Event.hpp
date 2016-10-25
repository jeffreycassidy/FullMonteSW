/*
 * Event.hpp
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_EVENT_HPP_
#define KERNELS_EVENT_HPP_

namespace KernelEvent {

enum Type {
	Launch,
	Boundary,
	Interface,
	ReflectInternal,
	ReflectFresnel,
	Refract,
	Absorb,
	Scatter,
	Exit,
	RouletteDie,
	RouletteWin,
	Abnormal,
	TimeGate,
	NoHit,
	Commit,
	Clear };
};


inline bool isPacketTermination(KernelEvent::Type e)
{
	return e == KernelEvent::RouletteDie || e == KernelEvent::Exit || e == KernelEvent::Abnormal || e == KernelEvent::TimeGate;
}

inline bool isPacketLaunch(KernelEvent::Type e)
{
	return e == KernelEvent::Launch;
}


#endif /* KERNELS_EVENT_HPP_ */
