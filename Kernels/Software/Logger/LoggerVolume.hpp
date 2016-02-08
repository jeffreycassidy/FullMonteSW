#pragma once
#include "Logger.hpp"
#include "AccumulationArray.hpp"

#include <FullMonte/OutputTypes/SpatialMapBase.hpp>

/*! Basic volume logger.
 *
 * Catches eventAbsorb() calls and accumulates weight in the appropriate tetra using template argument Accumulator
 *
 * Accumulator requirements:
 * 	Support operator[] returning some type T
 * 	T must support operator+= on Weight type
 * 	Copy-constructible
 * 	Constructor of type Accumulator(unsigned size,args...)
 *
 * Accumulator WorkerThread requirements:
 * 	Copy-constructible
 *
 *
 *	Examples: vector<T>& (single-thread), QueuedAccumulatorMT (thread-safe)
 */


template<class Accumulator>class LoggerVolume
{
	Accumulator acc;

public:
	LoggerVolume(){}
	LoggerVolume(LoggerVolume&& lv_) = delete;
	LoggerVolume(const LoggerVolume& lv_) = delete;

	class WorkerThread : public LoggerNull {
		typename Accumulator::WorkerThread wt;
	public:
		typedef void logger_member_tag;
		WorkerThread(Accumulator& parent_) : wt(parent_.get_worker()){};
		WorkerThread(const WorkerThread& wt_) = delete;
		WorkerThread(WorkerThread&& wt_) : wt(std::move(wt_.wt)){}
		~WorkerThread(){ wt.commit(); }

	    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventAbsorb(const Packet& pkt,unsigned IDt,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventCommit(){ wt.commit(); }
	};

	/// Access to the back-end accumulator
	Accumulator& accumulator();

	typedef WorkerThread ThreadWorker;

	WorkerThread get_worker() { return WorkerThread(acc);  };
};

