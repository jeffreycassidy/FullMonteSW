#ifndef KERNELS_SOFTWARE_LOGGER_LOGGER_HPP_
#define KERNELS_SOFTWARE_LOGGER_LOGGER_HPP_
#include <iostream>
#include <fstream>
#include <vector>
#include <type_traits>

#include <FullMonteSW/Geometry/newgeom.hpp>
#include <FullMonteSW/Geometry/Tetra.hpp>

#include "LoggerBase.hpp"

// Macro below is a convenience for defining new event types
// for DEFINE_EVENT(ename,emember) sets up so log_event(logger,<ename>,args...) calls logger.event<emember>(args...)

#define DEFINE_EVENT(ename,emember) typedef struct {} ename##_tag; \
	extern ename##_tag ename; \
	template<class Logger,typename... Args>inline typename std::enable_if< std::is_same<typename Logger::is_logger,std::true_type>::value,void>::type log_event(Logger& l,Events::ename##_tag,Args... args){ l.event##emember(args...); }

namespace Events {

// tags for event dispatch
// first arg is the tag name (eg. launch_tag), which is also the variable name (eg. Events::launch)
// the second arg creates a template that dispatches the event by default to a class member function (eg. logger.eventLaunch(args...) )
// for the default dispatch to work, you must typedef logger_member_tag to void in the class
DEFINE_EVENT(launch,Launch)
DEFINE_EVENT(absorb,Absorb)
DEFINE_EVENT(scatter,Scatter)
DEFINE_EVENT(boundary,Boundary)
DEFINE_EVENT(interface,Interface)
DEFINE_EVENT(reflect,ReflectInternal)
DEFINE_EVENT(refract,Refract)
DEFINE_EVENT(fresnel,ReflectFresnel)
DEFINE_EVENT(exit,Exit)
DEFINE_EVENT(newTetra,NewTetra)
DEFINE_EVENT(roulettewin,RouletteWin)
DEFINE_EVENT(roulettedie,Die)
DEFINE_EVENT(abnormal,Abnormal)
DEFINE_EVENT(timegate,TimeGate)
DEFINE_EVENT(nohit,NoHit)
DEFINE_EVENT(clear,Clear)

DEFINE_EVENT(commit,Commit);				// not actually an event but a request to the logger to sync its results globally

}

#endif

