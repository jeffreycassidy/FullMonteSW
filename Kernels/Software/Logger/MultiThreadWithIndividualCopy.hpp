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

#include "AbstractScorer.hpp"

template<typename T>void clear(T&);

/** MultiThreadWithIndividualCopy creates a Scorer from a simple single-threaded Logger type.
 *
 * It derives a new Logger type from the base, with an additional commit(AbstractScorer* S) method that merges
 * the results to the parent and clears the copy. Merging is protected by a mutex to avoid hazards, and by
 * default uses the += operator on the Logger::State type.
 *
 * Logger concept requirements:
 *
 * 		typedef State		State type held by the Logger
 *
 * 		Logger State concept requirements:
 *
 * 			operator+=(const State&)
 * 			void clear(State&)
 *
 * 			OutputData* createOutputData(const State&)			Package up the output data
 */

template<class SingleThreadLoggerT>class MultiThreadWithIndividualCopy : public AbstractScorer
{
public:
	MultiThreadWithIndividualCopy(){}

	/// The SingleThreadLoggerT with an added commit(AbstractScorer*) override
	class Logger;

	typedef SingleThreadLoggerT									SingleThreadLogger;
	typedef typename SingleThreadLoggerT::State					State;
	typedef MultiThreadWithIndividualCopy<SingleThreadLoggerT>	Parent;

	Logger get_logger(){ return Logger(); }

	virtual void clear() override;								///< Clear the values in the parent state
	virtual std::list<OutputData*> results() const override;	///< Package our state up and export it

	const State& state() const;

protected:
	void merge(State& st);

private:
	std::mutex	m_mutex;
	State		m_state;
};



template<class SingleThreadLoggerT>const typename SingleThreadLoggerT::State& MultiThreadWithIndividualCopy<SingleThreadLoggerT>::state() const
{
	return m_state;
}

template<class SingleThreadLoggerT>void MultiThreadWithIndividualCopy<SingleThreadLoggerT>::merge(typename SingleThreadLoggerT::State& st)
{
	{
		std::unique_lock<std::mutex> L(m_mutex);
		m_state += st;
	}
	::clear(st);
}

template<class SingleThreadLoggerT>void MultiThreadWithIndividualCopy<SingleThreadLoggerT>::clear()
{
	::clear(m_state);
}

template<class SingleThreadLoggerT>class MultiThreadWithIndividualCopy<SingleThreadLoggerT>::Logger : public SingleThreadLoggerT
{
	public:
		Logger(){}

		void commit(AbstractScorer& S)
		{
			auto& parent = static_cast<Parent&>(S);
			parent.merge(SingleThreadLoggerT::m_state);
		}
};

template<class SingleThreadLoggerT>std::list<OutputData*> MultiThreadWithIndividualCopy<SingleThreadLoggerT>::results() const
{
	return std::list<OutputData*>{createOutputData(state())};
}

#endif /* KERNELS_SOFTWARE_LOGGER_MULTITHREADWITHINDIVIDUALCOPY_HPP_ */
