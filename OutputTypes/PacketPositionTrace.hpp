/*
 * PacketTrace.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_PACKETPOSITIONTRACE_HPP_
#define OUTPUTTYPES_PACKETPOSITIONTRACE_HPP_

#include <array>
#include <vector>

#include <FullMonteSW/Kernels/Event.hpp>

/** Position trace of a single packet providing a series of steps (pos, weight, lengthTravelled, timeTravelled)
 *
 */

class PacketPositionTrace
{
public:
	typedef std::array<float,3> Point3;
	struct Step
	{
		Point3				pos;	///< Position

		float				w;		///< Weight (0..1)
		float				l;		///< Cumulative length traveled
		float 				t;		///< Time (simulation units)

		KernelEvent::Type 	event;
	};

	PacketPositionTrace();
	PacketPositionTrace(std::vector<Step>&& tr);

	~PacketPositionTrace();

	float 		length() 						const;	///< Physical length of the trace
	unsigned 	count() 						const;	///< Count of the number of steps
	float		duration() 						const;	///< Time duration of the trace

	Point3 		positionAtTime(float t) 		const;	///< Position after traveling a given amount of time
	Point3 		positionAfterLength(float l) 	const;	///< Position after traveling a given length

	const Step&	operator[](unsigned i)			const;	///< The i'th step

	std::vector<Step>::const_iterator begin() const { return m_trace.begin(); }
	std::vector<Step>::const_iterator end()   const { return m_trace.end();   }

private:
	static bool compareTime(const Step& s,float t);
	static bool compareLength(const Step& s,float l);

	std::vector<Step> m_trace;
};




#endif /* OUTPUTTYPES_PACKETPOSITIONTRACE_HPP_ */
