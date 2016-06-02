#ifndef KERNELS_SOFTWARE_LOGGER_LOGGERVOLUME_HPP_
#define KERNELS_SOFTWARE_LOGGER_LOGGERVOLUME_HPP_

#include "LoggerBase.hpp"
#include "AccumulationArray.hpp"

#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>

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
 * Accumulator ThreadWorker requirements:
 * 	Copy-constructible
 *
 *
 *	Examples: vector<T>& (single-thread), QueuedAccumulatorMT (thread-safe)
 */


template<class Accumulator>class LoggerVolume
{
public:
	LoggerVolume(){}
	LoggerVolume(unsigned Nt,unsigned Nq) : acc(Nt),m_Nq(Nq){}
	LoggerVolume(LoggerVolume&& lv_) = delete;
	LoggerVolume(const LoggerVolume& lv_) = delete;

	class ThreadWorker : public LoggerBase {
		typename Accumulator::ThreadWorker wt;
	public:

		typedef std::true_type is_logger;
		ThreadWorker(Accumulator& parent_,unsigned Nq) : wt(parent_.get_worker(Nq)){};
		ThreadWorker(const ThreadWorker& wt_) = delete;
		ThreadWorker(ThreadWorker&& wt_) : wt(std::move(wt_.wt)){}
		~ThreadWorker(){ /*wt.commit();*/ }

	    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventAbsorb(const Packet& pkt,unsigned IDt,double dw)
	    	{ wt[IDt] += dw; }

	    inline void eventCommit(){ wt.commit(); }
	};

	static std::list<OutputData*> results(const std::vector<double>& values)
	{
		// convert to float
		std::vector<float> vf(values.size());
		boost::copy(values, vf.begin());

		// create vector
		SpatialMapBase<float,unsigned> *vmap = SpatialMapBase<float,unsigned>::newFromVector(std::move(vf));
		OutputData* O = new VolumeAbsorbedEnergyMap(vmap);
		std::list<OutputData*> L;
		L.push_back(O);
		return L;
	}
	std::list<OutputData*> results() const { return results(acc.values()); }

	/// Access to the back-end accumulator
	Accumulator& accumulator();

	ThreadWorker get_worker() { return ThreadWorker(acc,m_Nq);  };

	void resize(unsigned N){ acc.resize(N); }
	void qSize(unsigned Nq){ m_Nq=Nq; }

private:
	Accumulator acc;
	unsigned m_Nq=1<<14;	///< Number of accumulations in queue
};

#endif
