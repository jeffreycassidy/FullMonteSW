#ifndef KERNELS_SOFTWARE_LOGGERS_ACCUMULATIONARRAY_HPP_
#define KERNELS_SOFTWARE_LOGGERS_ACCUMULATIONARRAY_HPP_
#include "Logger.hpp"
#include <FullMonteSW/Kernels/Software/Packet.hpp>
#include <FullMonteSW/OutputTypes/FluenceMapBase.hpp>
#include <type_traits>

/** Handles logging of surface exit events.
 *
 * @tparam 	Accumulator		Must support the AccumulatorConcept.
 */

template<class Accumulator>class LoggerSurface
{
public:
	typedef std::true_type is_logger;

	class ThreadWorker : public LoggerBase
	{
		typename Accumulator::ThreadWorker acc;
	public:
		typedef std::true_type is_logger;

		/// Construct from an Accumulator by getting a worker thread from the parent
		ThreadWorker(Accumulator& parent_,unsigned Nq) : acc(parent_.get_worker(Nq)){}

		/// Move constructor by simply moving the accumulator
		ThreadWorker(ThreadWorker&& wt_) : acc(std::move(wt_.acc)){}

		/// Copy constructor deleted
		ThreadWorker(const ThreadWorker& wt_) = delete;

		/// Commit results back to parent before deleting
		~ThreadWorker() { acc.commit(); }

		/// Record exit event by accumulating weight to the appropriate surface entry
		inline void eventExit(const Ray3,int IDf,double w){ acc[abs(IDf)] += w; }
		inline void eventExit(const Packet& pkt,int IDf){ acc[abs(IDf)] += pkt.w; }

		void eventCommit(){ acc.commit(); }
		inline void eventClear(){ acc.clear(); }
	};

	void resize(unsigned N){ acc.resize(N); }
	void qSize(unsigned Nq){ m_Nq=Nq; }

	/// Return a worker thread
	ThreadWorker get_worker() { return ThreadWorker(acc,m_Nq); }

	static std::list<OutputData*> results(const std::vector<double>& values)
	{
		// convert to float
		std::vector<float> se(values.size());
		boost::copy(values, se.begin());

		// create vector
		SpatialMapBase<float,unsigned> *smap = SpatialMapBase<float,unsigned>::newFromVector(std::move(se));
		OutputData* O = new SurfaceExitEnergyMap(smap);
		std::list<OutputData*> L;
		L.push_back(O);
		return L;
	}

	std::list<OutputData*> results() const { return results(acc.values()); }

	inline void eventClear()
	{
		clear();
	}

	void clear()
	{
		acc.clear();
	}

private:
	Accumulator acc;
	unsigned m_Nq=1<<14;
};

#endif

