/*
 * EventScorer.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_EVENTSCORER_HPP_
#define KERNELS_SOFTWARE_LOGGER_EVENTSCORER_HPP_

#include "LoggerWithState.hpp"

#include "MultiThreadWithIndividualCopy.hpp"

#include <FullMonteSW/OutputTypes/MCEventCounts.hpp>

class EventLogger :
	public LoggerWithState<MCEventCounts>
{
public:
    /** Event-specific overrides to implement AbstractLogger functions */
    inline void eventLaunch(AbstractScorer& S,const Ray3 r,unsigned IDt,double w)				{ ++m_state.Nlaunch; 	}

    inline void eventAbsorb(AbstractScorer& S,const Point3 p,unsigned IDt,double w0,double dw)	{ ++m_state.Nabsorb; 	}
    inline void eventScatter(AbstractScorer& S,const UVect3 d0,const UVect3 d,double g)			{ ++m_state.Nscatter; 	}

    inline void eventBoundary(AbstractScorer& S,const Point3 p,int,int,int)						{ ++m_state.Nbound; 	}

    inline void eventInterface(AbstractScorer& S,const Ray3,int,unsigned)						{ ++m_state.Ninterface;	}
    inline void eventRefract(AbstractScorer& S,const Point3,UVect3)								{ ++m_state.Nrefr; 		}
    inline void eventReflectInternal(AbstractScorer& S,const Point3,const UVect3)				{ ++m_state.Ntir; 		}
    inline void eventReflectFresnel(AbstractScorer& S,const Point3,UVect3)						{ ++m_state.Nfresnel; 	}

    inline void eventExit(AbstractScorer& S,const Ray3,int,double)								{ ++m_state.Nexit; 		}
    inline void eventDie(AbstractScorer& S,double)												{ ++m_state.Ndie; 		}
    inline void eventRouletteWin(AbstractScorer& S,double,double)								{ ++m_state.Nwin; 		}

    inline void eventAbnormal(AbstractScorer& S,const Packet&,unsigned,unsigned)				{ ++m_state.Nabnormal; 	}
};

typedef MultiThreadWithIndividualCopy<EventLogger> EventScorer;

#endif /* KERNELS_SOFTWARE_LOGGER_EVENTSCORER_HPP_ */
