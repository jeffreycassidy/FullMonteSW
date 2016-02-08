#pragma once
#include "Logger.hpp"
#include <FullMonte/Kernels/Software/Packet.hpp>

/** Handles logging of surface exit events.
 *
 * @tparam 	Accumulator		Must support the AccumulatorConcept.
 */

template<class Accumulator>class LoggerSurface {
	Accumulator acc;
public:

	typedef vector<typename Accumulator::ElementType> results_type;

	/** Construct and associate with a tetrahedral mesh.
	 * @param mesh_		Associated mesh, used to get the number of
	 * @param args...	Arguments to be passed through to the constructor for the underlying Accumulator type
	 */

	template<typename... Args>LoggerSurface(Args... args) : acc(args...){}
	LoggerSurface(LoggerSurface&& ls_) : acc(std::move(ls_.acc)){}

	/// Copy constructor deleted - no need for it
	LoggerSurface(const LoggerSurface& ls_) = delete;

	class WorkerThread : public LoggerNull {
		typename Accumulator::WorkerThread acc;
	public:
		typedef void logger_member_tag;
		/// Construct from an Accumulator by getting a worker thread from the parent
		WorkerThread(Accumulator& parent_) : acc(parent_.get_worker()){}

		/// Move constructor by simply moving the accumulator
		WorkerThread(WorkerThread&& wt_) : acc(std::move(wt_.acc)){}

		/// Copy constructor deleted
		WorkerThread(const WorkerThread& wt_) = delete;

		/// Commit results back to parent before deleting
		~WorkerThread() { acc.commit(); }

		/// Record exit event by accumulating weight to the appropriate surface entry
		inline void eventExit(const Ray3,int IDf,double w){ acc[abs(IDf)] += w; }
		inline void eventExit(const Packet& pkt,int IDf){ acc[abs(IDf)] += pkt.w; }

		void eventCommit(){ acc.commit(); }
	};

	typedef WorkerThread ThreadWorker;

	/// Merge a worker thread's partial results
	LoggerSurface& operator+=(const WorkerThread& wt_){ wt_.commit(); return *this; }

	/// Return a worker thread
	WorkerThread get_worker() { return WorkerThread(acc); }

	template<typename T>friend ostream& operator<<(ostream& os,const LoggerSurface<T>& ls);
};
