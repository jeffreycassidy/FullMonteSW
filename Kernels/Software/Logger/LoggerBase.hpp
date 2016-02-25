/*
 * LoggerBase.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_LOGGERBASE_HPP_
#define KERNELS_SOFTWARE_LOGGER_LOGGERBASE_HPP_

class Packet;
class OutputData;
#include <list>

class LoggerBase
{
public:
	typedef struct LoggerBaseNullState {} State;
	const State& state() const { return m_nullstate; }

	LoggerBase(){}							///< Default constructor does nothing
	LoggerBase(LoggerBase&& lm_){}			///< Move constructor does nothing
	LoggerBase(const LoggerBase& lm_){}		///< Copy constructor does nothing
	LoggerBase& operator=(const LoggerBase& lm_){ return *this; }
    virtual ~LoggerBase(){}

    typedef __m128 Point3;
    typedef __m128 UVect3;
    typedef pair<__m128,__m128> Ray3;

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w){};

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw){};
    inline void eventScatter(const UVect3 d0,const UVect3 d,double g){};

    inline void eventBoundary(const Point3 p,int,int,int){};        // boundary (same material)

    inline void eventInterface(const Ray3,int,unsigned){};          // found a material interface; possible results are:
    inline void eventRefract(const Point3,UVect3){};                //      refracted
    inline void eventReflectInternal(const Point3,const UVect3){};  //      internal reflection
    inline void eventReflectFresnel(const Point3,UVect3){};         //      fresnel reflection

    inline void eventNewTetra(const Packet& pkt,const Tetra& T0,const unsigned tetraFaceIndex){};
    	// packet pkt propagating from tetra T0 from face (tetraFaceIndex) 0..3

    // termination events
    inline void eventExit(const Ray3,int,double){};            // exited geometry
    inline void eventDie(double){};                                     // lost Russian roulette
    inline void eventRouletteWin(double,double){};

    inline void eventAbnormal(const Packet&,unsigned,unsigned){};

    inline void eventTimeGate(const Packet&){};						// Exceeded time gate
    inline void eventNoHit(const Packet&,const Tetra&){};			// No hit in intersection

    inline void eventCommit(){};
    inline void eventClear(){};

    static std::list<OutputData*> results(const State& st)
	{
    	return std::list<OutputData*>();
	}
    std::list<OutputData*> results() const { return results(m_nullstate); }

private:
	State m_nullstate;
};

#endif /* KERNELS_SOFTWARE_LOGGER_LOGGERBASE_HPP_ */
