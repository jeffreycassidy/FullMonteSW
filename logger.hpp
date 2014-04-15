#ifndef LOGGER_INCLUDED
#define LOGGER_INCLUDED
#include <iostream>
#include <fstream>
#include <vector>
#include "graph.hpp"
#include "newgeom.hpp"
#include "fluencemap.hpp"

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

template<typename... LoggerTs>class LoggerMulti;

template<typename Ta,typename Tb>ostream& operator<<(ostream& os,const LoggerMulti<Ta,Tb>& lm)
{
	return os << lm.head.getResults() << endl << lm.tail.getResults() << endl;
}

template<typename... Ts>ostream& operator<<(ostream& os,const LoggerMulti<Ts...>& lm)
{
	return os << lm.head.getResults() << endl << lm.tail;
}

class LoggerNull {
    public:
	LoggerNull(){}							///< Default constructor does nothing
	LoggerNull(LoggerNull&& lm_){}			///< Move constructor does nothing
	LoggerNull(const LoggerNull& lm_){}		///< Copy constructor does nothing
	LoggerNull& operator=(const LoggerNull& lm_){ return *this; }
    virtual ~LoggerNull(){}

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w){};

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
    inline void eventRouletteWin(double,double){};                      // won roulette

    typedef LoggerNull ThreadWorker;

    //ThreadWorker getThreadWorkerInstance(unsigned) const { return ThreadWorker(); }
    LoggerNull get_worker() { return LoggerNull(); }

    const LoggerNull& operator+=(const LoggerNull&){ return *this; }
    LoggerNull operator+(const LoggerNull&){ return LoggerNull(); }
};

template<typename LoggerH,typename LoggerT>class LoggerMulti<LoggerH,LoggerT> {
	LoggerH head;
	LoggerT tail;

	typedef typename LoggerH::ThreadWorker HeadWorker;
	typedef typename LoggerT::ThreadWorker TailWorker;

public:
	typedef LoggerMulti<HeadWorker,TailWorker> ThreadWorker;
	LoggerMulti(LoggerH&& head_,LoggerT&& tail_) : head(std::move(head_)),tail(std::move(tail_)){}
	LoggerMulti(const LoggerH& head_,const LoggerT& tail_) : head(head_),tail(tail_){}
	LoggerMulti(LoggerMulti&& lm_) : head(std::move(lm_.head)),tail(std::move(lm_.tail)){}

	virtual ~LoggerMulti(){};

    // event handlers
    inline void eventLaunch(const Ray3 r,unsigned IDt,double w)
        { head.eventLaunch(r,IDt,w); tail.eventLaunch(r,IDt,w); };

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw)
        { head.eventAbsorb(p,IDt,w0,dw); tail.eventAbsorb(p,IDt,w0,dw); };

    inline void eventScatter(const UVect3 d0,const UVect3 d,double g)
        { head.eventScatter(d0,d,g); tail.eventScatter(d0,d,g); };

    inline void eventBoundary(const Point3 p,int IDf,int IDts,int IDte)        // boundary (same material)
        { head.eventBoundary(p,IDf,IDts,IDte); tail.eventBoundary(p,IDf,IDts,IDte); }

    inline void eventInterface(const Ray3 r,int IDf,unsigned a)          // found a material interface; possible results are:
        { head.eventInterface(r,IDf,a); tail.eventInterface(r,IDf,a); }

    inline void eventRefract(const Point3 p,UVect3 d)                //      refracted
        { head.eventRefract(p,d); tail.eventRefract(p,d); }

    inline void eventReflectInternal(const Point3 p,const UVect3 d)  //      internal reflection
        { head.eventReflectInternal(p,d); tail.eventReflectInternal(p,d); }

    inline void eventReflectFresnel(const Point3 p,UVect3 d)         //      fresnel reflection
        { head.eventReflectFresnel(p,d); tail.eventReflectFresnel(p,d); }

    // termination events
    inline void eventExit(const Ray3 r,int IDf,double w)            // exited geometry
        { head.eventExit(r,IDf,w); tail.eventExit(r,IDf,w); }

    inline void eventDie(double w)                                     // lost Russian roulette
        { head.eventDie(w); tail.eventDie(w); }

    inline void eventRouletteWin(double w0,double w)                      // won roulette
        { head.eventRouletteWin(w0,w); tail.eventRouletteWin(w0,w); }

    const LoggerMulti& operator+=(const LoggerMulti& rhs)
        { head += rhs.head; tail += rhs.tail; return *this; }

    /*const LoggerMulti& operator+=(const ThreadWorker& rhs)
            { head += rhs.head; tail += rhs.tail; return *this; }*/

    ThreadWorker get_worker(){
    	return ThreadWorker(head.get_worker(),tail.get_worker());
    }

    friend ostream& operator<<<>(ostream&,const LoggerMulti&);
};

template<typename LoggerH,typename... LoggerTs>class LoggerMulti<LoggerH,LoggerTs...> {
	LoggerH head;
	LoggerMulti<LoggerTs...> tail;

	typedef typename LoggerH::ThreadWorker HeadWorker;
	typedef decltype(tail.get_worker()) TailWorker;

public:
	typedef LoggerMulti<HeadWorker,TailWorker> ThreadWorker;
	//LoggerMulti(Logger&& head_) : head(std::move(head_)),tail(LoggerMulti<>()){}
	LoggerMulti(LoggerH&& head_,LoggerTs&&... tail_) : head(std::move(head_)),tail(std::move(LoggerMulti<LoggerTs...>(std::forward<LoggerTs>(tail_)...))){}
	//LoggerMulti(const LoggerH& head_) : head(head_){}
	//LoggerMulti(const LoggerH& head_,const LoggerTs&... tail_) : head(head_),tail(tail_...){}
	//LoggerMulti(Logger&& head_,Loggers&&... tail_) : head(std::move(head_)),tail(LoggerMulti<Loggers...>(std::move(tail_...))) {};
	LoggerMulti(LoggerMulti&& lm_) : head(std::move(lm_.head)),tail(std::move(lm_.tail)){}

	virtual ~LoggerMulti(){};

    // event handlers
    inline void eventLaunch(const Ray3 r,unsigned IDt,double w)
        { head.eventLaunch(r,IDt,w); tail.eventLaunch(r,IDt,w); };

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw)
        { head.eventAbsorb(p,IDt,w0,dw); tail.eventAbsorb(p,IDt,w0,dw); };

    inline void eventScatter(const UVect3 d0,const UVect3 d,double g)
        { head.eventScatter(d0,d,g); tail.eventScatter(d0,d,g); };

    inline void eventBoundary(const Point3 p,int IDf,int IDts,int IDte)        // boundary (same material)
        { head.eventBoundary(p,IDf,IDts,IDte); tail.eventBoundary(p,IDf,IDts,IDte); }

    inline void eventInterface(const Ray3 r,int IDf,unsigned a)          // found a material interface; possible results are:
        { head.eventInterface(r,IDf,a); tail.eventInterface(r,IDf,a); }

    inline void eventRefract(const Point3 p,UVect3 d)                //      refracted
        { head.eventRefract(p,d); tail.eventRefract(p,d); }

    inline void eventReflectInternal(const Point3 p,const UVect3 d)  //      internal reflection
        { head.eventReflectInternal(p,d); tail.eventReflectInternal(p,d); }

    inline void eventReflectFresnel(const Point3 p,UVect3 d)         //      fresnel reflection
        { head.eventReflectFresnel(p,d); tail.eventReflectFresnel(p,d); }

    // termination events
    inline void eventExit(const Ray3 r,int IDf,double w)            // exited geometry
        { head.eventExit(r,IDf,w); tail.eventExit(r,IDf,w); }

    inline void eventDie(double w)                                     // lost Russian roulette
        { head.eventDie(w); tail.eventDie(w); }

    inline void eventRouletteWin(double w0,double w)                      // won roulette
        { head.eventRouletteWin(w0,w); tail.eventRouletteWin(w0,w); }

    const LoggerMulti& operator+=(const LoggerMulti& rhs)
        { head += rhs.head; tail += rhs.tail; return *this; }

    const LoggerMulti& operator+=(const ThreadWorker& rhs){
    	head += rhs.head; tail += rhs.tail; return *this; }

    ThreadWorker get_worker(){
    	return ThreadWorker(head.get_worker(),tail.get_worker());
    }

    friend ostream& operator<<<>(ostream&,const LoggerMulti&);
};




#endif
