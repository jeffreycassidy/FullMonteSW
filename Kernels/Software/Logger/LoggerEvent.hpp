#ifndef KERNELS_SOFTWARE_LOGGERS_LOGGEREVENT_HPP
#define KERNELS_SOFTWARE_LOGGERS_LOGGEREVENT_HPP
#include "Logger.hpp"
#include <mutex>

#include <FullMonteSW/OutputTypes/MCEventCounts.hpp>

/** LoggerEvent counts events as they happen.
 *
 */

class LoggerEvent : public LoggerBase
{
public:
    LoggerEvent(){ }
    LoggerEvent(LoggerEvent&& le_) : m_events(le_.m_events)
    	{ le_.clear(); }

    // Logger concept requirements
	typedef MCEventCounts State;
	const State& state() const { return m_events; }

    void clear()
    {
    	m_events=MCEventCounts();
    }

    static std::list<OutputData*> results(const State& st){ return std::list<OutputData*>(1, new MCEventCountsOutput(st)); }
    std::list<OutputData*> results() const { return results(m_events); }


    // Event-specific overrides

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w)					{ ++m_events.Nlaunch; 	}	// launch new packet

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw)	{ ++m_events.Nabsorb; 	} 	// packet absorbed
    inline void eventScatter(const UVect3 d0,const UVect3 d,double g)			{ ++m_events.Nscatter; 	}

    inline void eventBoundary(const Point3 p,int,int,int)						{ ++m_events.Nbound; 	}   // boundary (same material)
    inline void eventInterface(const Ray3,int,unsigned)							{ ++m_events.Ninterface;} 	// found a material interface; possible results are:
    inline void eventRefract(const Point3,UVect3)								{ ++m_events.Nrefr; 	}   	//      refracted
    inline void eventReflectInternal(const Point3,const UVect3)					{ ++m_events.Ntir; 		}       //      internal reflection
    inline void eventReflectFresnel(const Point3,UVect3)						{ ++m_events.Nfresnel; 	}       //      fresnel reflection

    inline void eventExit(const Ray3,int,double)								{ ++m_events.Nexit; 	} 	// exited geometry
    inline void eventDie(double)												{ ++m_events.Ndie; 		}  	// lost Russian roulette
    inline void eventRouletteWin(double,double)									{ ++m_events.Nwin; 		}   // won roulette

    inline void eventAbnormal(const Packet&,unsigned,unsigned)					{ ++m_events.Nabnormal; }

    inline void eventClear(){ clear(); }

private:
    MCEventCounts m_events;
};

#endif

