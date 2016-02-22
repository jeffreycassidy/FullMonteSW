/*
 * MultiThreadWithIndividualCopy.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_MULTITHREADWITHINDIVIDUALCOPY_HPP_
#define KERNELS_SOFTWARE_LOGGER_MULTITHREADWITHINDIVIDUALCOPY_HPP_

#include <mutex>
#include <type_traits>

/** Converts a single-thread logger into a multi-threaded one.
 *
 * SingleThreadLogger:
 * 		meets requirements of Logger
 * 			clear() clears state
 * 			state() returns a const& to the current state
 * 			default-constructible
 *
 * Parent:
 * 		merge(SingleThreadLogger::State&) adds results in a thread-safe manner
 * 		get_worker() returns a ThreadWorker referenced to this object
 * 						derived from SingleThreadLogger and handling the same events
 * 						call to commit() calls parent's merge method, then clears the state
 */


template<class SingleThreadLogger>class MultiThreadWithIndividualCopy
{
public:
	MultiThreadWithIndividualCopy(){}
	MultiThreadWithIndividualCopy(MultiThreadWithIndividualCopy&& base) :
		m_state(std::move(base.m_state)),
		m_mutex()
	{}

	typedef typename SingleThreadLogger::State					State;
	typedef MultiThreadWithIndividualCopy<SingleThreadLogger> 	Parent;

	class ThreadWorker : public SingleThreadLogger
	{
	public:
		ThreadWorker(Parent& p) : m_parent(p){}
		ThreadWorker(const ThreadWorker& t) : m_parent(t.m_parent){}

		typedef std::true_type is_logger;

		~ThreadWorker(){ /*commit();*/ }

		void commit()
		{
			m_parent.merge(this->SingleThreadLogger::state());
			SingleThreadLogger::clear();
		}

		void eventCommit(){ commit(); }

	private:
		Parent&		m_parent;
	};


	ThreadWorker get_worker(){ return ThreadWorker(*this); }

	void merge(const State& st)
	{
		std::unique_lock<std::mutex> L(m_mutex);
		m_state += st;
	}

	std::list<OutputData*> results() const { return	SingleThreadLogger::results(m_state); }

private:
	std::mutex	m_mutex;
	State		m_state;
};





#endif /* KERNELS_SOFTWARE_LOGGER_MULTITHREADWITHINDIVIDUALCOPY_HPP_ */
