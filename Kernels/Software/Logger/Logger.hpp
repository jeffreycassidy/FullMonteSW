#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <type_traits>

#include <FullMonte/Kernels/Software/Packet.hpp>
#include <FullMonte/Geometry/Tetra.hpp>
#include <FullMonte/Geometry/TetraMesh.hpp>

// Macro below is a convenience for defining new event types
// for DEFINE_EVENT(ename,emember) sets up so log_event(logger,<ename>,args...) calls logger.event<emember>(args...)

#define DEFINE_EVENT(ename,emember) typedef struct {} ename##_tag; \
	extern ename##_tag ename; \
	template<class Logger,typename... Args>typename Logger::logger_member_tag log_event(Logger& l,Events::ename##_tag,Args... args){ l.event##emember(args...); }

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
DEFINE_EVENT(roulettewin,RouletteWin)
DEFINE_EVENT(roulettedie,Die)
DEFINE_EVENT(abnormal,Abnormal)
DEFINE_EVENT(timegate,TimeGate)
DEFINE_EVENT(nohit,NoHit)

DEFINE_EVENT(commit,Commit);				// not actually an event but a request to the logger to sync its results globally

}

// set default to nop for all events (dangerous, may miss events unexpectedly if give bad args)
//template<class EventTag,typename... Args>void log_event(LoggerNull&,EventTag,Args...){}

typedef __m128 Point3;
typedef __m128 UVect3;
typedef pair<__m128,__m128> Ray3;

/// LoggerNull defines the basic functions that a Logger can overload

/* Logger policies
 *
 * 1) local copies, merge at end			Needs + / +=
 * 2) central copy, queued atomic merge		Needs merge
 * 		(null merge at end)
 * 3) central copy, free-for-all			Just a non-const ref (not thread safe)
 * 		(null merge at end)
 * 4) local copies, different behaviors		Things like memtrace, which runs per-core
 * 		(null merge)
 *
 * Or just implement all policies at once?
 *
 */

// Function object which any type T with operator<< defined out to provided ostream
template<class OS>class ostream_functor {
	OS& os;
public:
	ostream_functor(OS& os_) : os(os_){}

	template<typename T>OS& operator()(const T& t){ return os<<t; }
};

// Good idea from Stack Overflow
template<std::size_t I=0,typename FuncT,typename... Tp>inline typename std::enable_if<I==sizeof...(Tp),void>::type
		tuple_for_each(const std::tuple<Tp...>&,FuncT){}

template<std::size_t I=0,typename FuncT,typename... Tp>inline typename std::enable_if<I < sizeof...(Tp),void>::type
		tuple_for_each(const std::tuple<Tp...>& t,FuncT f)
{
	f(std::get<I>(t));
	tuple_for_each<I+1,FuncT,Tp...>(t,f);
}

template<typename... Loggers>ostream& operator<<(ostream& os,const tuple<Loggers...>& l)
{
	ostream_functor<ostream> f(os);
	tuple_for_each(l,f);
	return os;
}


// Defines get_worker
// TODO: Clean this up (use variadic args); probably needs a helper class to get the types right without infinite recursion

template<class LA>std::tuple<typename LA::ThreadWorker> get_worker(std::tuple<LA>& t)
		{ return make_tuple(get<0>(t).get_worker()); }

template<class LA,class LB>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker> get_worker(std::tuple<LA,LB>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker()); }

template<class LA,class LB,class LC>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker,typename LC::ThreadWorker> get_worker(std::tuple<LA,LB,LC>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker(),get<2>(t).get_worker()); }

template<class LA,class LB,class LC,class LD>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker,typename LC::ThreadWorker,typename LD::ThreadWorker> get_worker(std::tuple<LA,LB,LC,LD>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker(),get<2>(t).get_worker(),get<3>(t).get_worker()); }

template<class LA,class LB,class LC,class LD,class LE>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker,typename LC::ThreadWorker,typename LD::ThreadWorker,typename LE::ThreadWorker> get_worker(std::tuple<LA,LB,LC,LD,LE>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker(),get<2>(t).get_worker(),get<3>(t).get_worker(),get<4>(t).get_worker()); }


class LoggerNull {
    public:
	LoggerNull(){}							///< Default constructor does nothing
	LoggerNull(LoggerNull&& lm_){}			///< Move constructor does nothing
	LoggerNull(const LoggerNull& lm_){}		///< Copy constructor does nothing
	LoggerNull& operator=(const LoggerNull& lm_){ return *this; }
    virtual ~LoggerNull(){}

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w){};

    typedef __m128 Point3;
    typedef __m128 UVect3;
    typedef pair<__m128,__m128> Ray3;

    /// LoggerNull defines the basic functions that a Logger can overload

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw){};
    inline void eventScatter(const UVect3 d0,const UVect3 d,double g){};

    inline void eventBoundary(const Point3 p,int,int,int){};        // boundary (same material)

    inline void eventInterface(const Ray3,int,unsigned){};          // found a material interface; possible results are:
    inline void eventRefract(const Point3,UVect3){};                //      refracted
    inline void eventReflectInternal(const Point3,const UVect3){};  //      internal reflection
    inline void eventReflectFresnel(const Point3,UVect3){};         //      fresnel reflection

    // termination events
    inline void eventExit(const Ray3,int,double){};            // exited geometry
    inline void eventDie(double){};                                     // lost Russian roulette
    inline void eventRouletteWin(double,double){};

    inline void eventAbnormal(const Packet&,unsigned,unsigned){};

    inline void eventTimeGate(const Packet&){};						// Exceeded time gate
    inline void eventNoHit(const Packet&,const Tetra&){};			// No hit in intersection

    inline void eventCommit(){};

    typedef LoggerNull ThreadWorker;

    LoggerNull get_worker() { return LoggerNull(); }
//
//    const LoggerNull& operator+=(const LoggerNull&){ return *this; }
//    LoggerNull operator+(const LoggerNull&){ return LoggerNull(); }
};


// dispatches the event to all members of a tuple
template<std::size_t I=0,class LoggerTuple,class EventTag,typename... Args>inline
	typename std::enable_if<( I < tuple_size<LoggerTuple>::value),void>::type log_event(LoggerTuple& l,EventTag e,Args... args)
{
	log_event(get<I>(l),e,args...);
	log_event<I+1>(l,e,args...);
}

// base case (I==tuple size, ie. one past end so do nothing)
template<std::size_t I,class LoggerTuple,class EventTag,typename... Args>inline
	typename std::enable_if<(I==tuple_size<LoggerTuple>::value),void>::type log_event(LoggerTuple& l,EventTag e,Args... args)
{}
