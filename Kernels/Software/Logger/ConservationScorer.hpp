/*
 * ConservationScorer.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_CONSERVATIONSCORER_HPP_
#define KERNELS_SOFTWARE_LOGGER_CONSERVATIONSCORER_HPP_

#include "MultiThreadWithIndividualCopy.hpp"

#include <FullMonteSW/OutputTypes/MCConservationCounts.hpp>

#include "LoggerWithState.hpp"

#include <FullMonteSW/Kernels/Software/Packet.hpp>

class Packet;

/** Accumulates the disposition of energy to ensure that all packet weight is accounted for.
 *
 * LoggerWithState<...> ensures compliance with Logger concept.
 */

class ConservationLogger : public LoggerWithState<MCConservationCounts>
{
public:
	inline void eventLaunch(AbstractScorer&,Ray3 r,unsigned IDt,double w) 				{ m_state.w_launch += w; 				}
	inline void eventAbsorb(AbstractScorer&,Point3 p,unsigned IDt,double w0,double dw) 	{ m_state.w_absorb += dw; 				}
	inline void eventExit(AbstractScorer&,Ray3 r,int IDf,double w) 						{ m_state.w_exit += w; 					}
	inline void eventDie(AbstractScorer&,double w)										{ m_state.w_die += w; 					}
	inline void eventRouletteWin(AbstractScorer&,double w0,double w)					{ m_state.w_roulette += w-w0; 			}

	inline void eventAbnormal(AbstractScorer&,const Packet& pkt,unsigned,unsigned)		{ m_state.w_abnormal += pkt.weight(); 	}

	inline void eventNoHit(AbstractScorer&,const Packet& pkt,const Tetra&)				{ m_state.w_nohit += pkt.weight(); 		}
	inline void eventTimeGate(AbstractScorer&,const Packet& pkt)						{ m_state.w_time += pkt.weight(); 		}
};

typedef MultiThreadWithIndividualCopy<ConservationLogger> ConservationScorer;

#endif /* KERNELS_SOFTWARE_LOGGER_CONSERVATIONSCORER_HPP_ */
