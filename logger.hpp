#ifndef LOGGER_INCLUDED
#define LOGGER_INCLUDED
#include <iostream>
#include <fstream>
#include <vector>
#include "graph.hpp"
#include "newgeom.hpp"
#include "fluencemap.hpp"
#include <boost/thread.hpp>

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

template<typename... Loggers>class LoggerMulti;

template<typename Logger,typename... Loggers>class LoggerMulti<Logger,Loggers...> {
	Logger head;
	LoggerMulti<Loggers...> tail;

	typedef typename Logger::ThreadWorker HeadWorker;
	typedef decltype(tail.get_worker()) TailWorker;

public:
	typedef LoggerMulti<HeadWorker,TailWorker> ThreadWorker;
	//LoggerMulti(Logger&& head_) : head(std::move(head_)),tail(LoggerMulti<>()){}
	LoggerMulti(Logger&& head_,Loggers&&... tail_) : head(std::move(head_)),tail(LoggerMulti<Loggers...>(std::move(tail_...))) {};
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

    LoggerMulti<HeadWorker,TailWorker> get_worker(){
    	return LoggerMulti<HeadWorker,TailWorker>(head.get_worker(),tail.get_worker());
    }

    //template<class H0,class T0>friend ostream& operator<<(ostream&,const LoggerCons<H0,T0>& l);
};

template<>class LoggerMulti<> {
    public:
	LoggerMulti(){}
	LoggerMulti(LoggerMulti&& lm_){}
	LoggerMulti(const LoggerMulti& lm_){}
	LoggerMulti& operator=(const LoggerMulti& lm_){ return *this; }
    virtual ~LoggerMulti(){}

	typedef LoggerMulti<> HeadWorker;
	typedef LoggerMulti<> TailWorker;

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

    typedef LoggerMulti<> ThreadWorker;

    ThreadWorker getThreadWorkerInstance(unsigned) const { return ThreadWorker(); }
    LoggerMulti<> get_worker() { return LoggerMulti<>(); }

    const LoggerMulti& operator+=(const LoggerMulti&){ return *this; }
    LoggerMulti operator+(const LoggerMulti&){ return LoggerMulti(); }
};

/// The Logger for a null_type is a null logger: no events
typedef class LoggerMulti<> LoggerNull;

/*template<unsigned N,typename Tuple>struct GetWorkers {
    static auto map_it(F& f,const Tuple& t) -> decltype(tuple_cat(TupleMap<N-1,Tuple,F>::map_it(f,t),make_tuple(f(get<N>(t))))){
        return tuple_cat(TupleMap<N-1,Tuple,F>::map_it(f,t),make_tuple(f(get<N>(t))));
    }
};

template<unsigned N,typename Tuple>struct GetWorkers;

template<typename Tuple,class F>struct GetWorkers<0,Tuple,F> {
    static auto map_it(F& f,const Tuple& t) -> decltype(make_tuple(f(get<0>(t)))) {
        return make_tuple(f(get<0>(t)));
    }
};*/

/*template<typename Tuple,typename F>auto map_my_tuple(F& f,Tuple& t) -> decltype(TupleMap<tuple_size<Tuple>::value-1,Tuple,F>::map_it(f,t))
{
    auto ret = TupleMap<tuple_size<Tuple>::value-1,Tuple,F>::map_it(f,t);
    return ret;
}*/


/* Example mapper; must have an operator() for each type to be processed
    Currently has a default which gives identity
    */

/*struct my_mapper {
    template<class T>T operator()(T i){ return i; }
    char*  operator()(char* p){ return p; }
//    string operator()(int i){ stringstream ss; ss << "s(i):" << i; return ss.str(); }
//    string operator()(unsigned u){ stringstream ss; ss << "s(u):" << u; return ss.str(); }
    string operator()(string s){ return s; }
    string operator()(unsigned long i){ stringstream ss; ss << "s(u):" << i; return ss.str(); }
    string operator()(double d){ stringstream ss; ss << "s(d):" << setprecision(3) << fixed << d; return ss.str(); }
};*/

// Buffer is a support class

/*
template<class T>class Buffer {
    protected:
    T*          first;
    T*          last;
    T*          current;    // always points to a valid place [first.get() .. last-1]

    unsigned N;

    virtual void atBufferEnd(const T*,const T*)=0;

    public:

    Buffer(unsigned N_,bool doInit_) :
        first(new T[N_]),
        last(first+N_),
        current(doInit_ ? last-1 : first),
        N(N_){}

    Buffer(Buffer&& b_) : first(b_.first),last(b_.last),current(b_.current),N(b_.N){
        b_.first = b_.current = NULL;
        b_.N = 0;
    }

    virtual ~Buffer() { delete[] first; }

	void flush() { atBufferEnd(first,current); current=first; }

    // get next available slot; if at last slot, flush buffer and go back to start
    T* getNext(){
        if (current == last-1)
        {
            atBufferEnd(first,last);
            current=first;
        }
		else
			++current;
        return current;
    }
};*/


#endif
