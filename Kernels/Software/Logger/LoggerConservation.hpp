#ifndef KERNELS_SOFTWARE_LOGGERS_LOGGERCONSERVATION_HPP_
#define KERNELS_SOFTWARE_LOGGERS_LOGGERCONSERVATION_HPP_
#include "LoggerBase.hpp"
#include <FullMonteSW/Kernels/Software/Packet.hpp>
#include <FullMonteSW/OutputTypes/MCConservationCounts.hpp>

/** Single-threaded conservation tracker.
 *
 */

class LoggerConservation : public LoggerBase
{
public:
	typedef MCConservationCounts State;

	/// Initialize to zero
	LoggerConservation(){};

	inline void eventLaunch(Ray3 r,unsigned IDt,double w) 				{ m_counts.w_launch += w; 				}
	inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw) 	{ m_counts.w_absorb += dw; 				}
	inline void eventExit(Ray3 r,int IDf,double w) 						{ m_counts.w_exit += w; 				}
	inline void eventDie(double w)										{ m_counts.w_die += w; 					}
	inline void eventRouletteWin(double w0,double w)					{ m_counts.w_roulette += w-w0; 			}

	inline void eventAbnormal(const Packet& pkt,unsigned,unsigned)		{ m_counts.w_abnormal += pkt.weight(); 	}

	inline void eventNoHit(const Packet& pkt,const Tetra&)				{ m_counts.w_nohit += pkt.weight(); 	}
	inline void eventTimeGate(const Packet& pkt)						{ m_counts.w_time += pkt.weight(); 		}

	void clear() { *this = LoggerConservation(); }

	inline void eventClear(){ clear(); }

	const State& state() const { return m_counts; }

	static std::list<OutputData*> results(const State& st)
	{
		std::list<OutputData*> o;
		o.push_back(new MCConservationCountsOutput(st));
		return o;
	}
	std::list<OutputData*> results() const { return results(m_counts); }

private:
	MCConservationCounts	m_counts;
};

#endif

