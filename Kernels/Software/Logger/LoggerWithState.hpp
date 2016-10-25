/*
 * LoggerWithState.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_LoggerWithState_HPP_
#define KERNELS_SOFTWARE_LOGGER_LoggerWithState_HPP_

class Packet;

#include "BaseLogger.hpp"


/// A default implementation of clear(): use the object's own method
template<class StateT>void clear(StateT& st)
{
	st.clear();
}


/** LoggerWithState encapsulates the basic idea of a Logger that has some kind of State class built it.
 * It provides read-only access via state(), a method to reset the contents via clear()
 *
 * Concept requirements for the state class:
 *
 * 		void clear(State& st)		Should reset to a default state
 *
 */

template<class StateT>class LoggerWithState : public BaseLogger
{
public:
	typedef StateT State;

	LoggerWithState();
	~LoggerWithState();

	void clear();

	// Derived classes should override commit(S) below if needed to implement their functionality
	//void commit(AbstractScorer* S);

	const State& state() const;

protected:
	State m_state;
};

template<class StateT>LoggerWithState<StateT>::LoggerWithState()
{
}

template<class StateT>LoggerWithState<StateT>::~LoggerWithState()
{
}

template<class StateT>void LoggerWithState<StateT>::clear()
{
	::clear(m_state);
}

template<class StateT>const StateT& LoggerWithState<StateT>::state() const
{
	return m_state;
}

#endif /* KERNELS_SOFTWARE_LOGGER_LoggerWithState_HPP_ */
