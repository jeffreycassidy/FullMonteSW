/*
 * LoggerTuple.hpp
 *
 *  Created on: Feb 8, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_LOGGERTUPLE_HPP_
#define KERNELS_SOFTWARE_LOGGER_LOGGERTUPLE_HPP_

#include <tuple>
#include <type_traits>

/** log_event definition for tuple of Loggers std::tuple<L0,L1,...>: just dispatch to each one by calling log_event for each element */

// dispatches the event to all members of a tuple
template<std::size_t I=0,class LoggerTuple,class ScorerTuple,class EventTag,typename... Args>inline
	typename std::enable_if<( I < std::tuple_size<LoggerTuple>::value),void>::type log_event(LoggerTuple& l,ScorerTuple& s,EventTag e,Args... args)
{
	log_event(get<I>(l),get<I>(s),e,args...);
	log_event<I+1>(l,s,e,args...);
}

// base case (I==tuple size, ie. one past end so do nothing)
template<std::size_t I,class LoggerTuple,class ScorerTuple,class EventTag,typename... Args>inline
	typename std::enable_if<(I==std::tuple_size<LoggerTuple>::value),void>::type log_event(LoggerTuple& l,ScorerTuple& s,EventTag e,Args... args)
{
}



// dispatches the event to all members of a tuple
template<std::size_t I=0,class LoggerTuple,class ScorerTuple>inline
	typename std::enable_if<( I < std::tuple_size<LoggerTuple>::value),void>::type commit(LoggerTuple& l,ScorerTuple& s)
{
	get<I>(l).commit(get<I>(s));
	commit<I+1>(l,s);
}

// base case (I==tuple size, ie. one past end so do nothing)
template<std::size_t I,class LoggerTuple,class ScorerTuple>inline
	typename std::enable_if<(I==std::tuple_size<LoggerTuple>::value),void>::type commit(LoggerTuple& l,ScorerTuple& s)
{
}


// dispatches the event to all members of a tuple
template<std::size_t I=0,class... Loggers>inline
	typename std::enable_if<( I < std::tuple_size<std::tuple<Loggers...>>::value),void>::type clear(std::tuple<Loggers...>& l)
{
	get<I>(l).clear();
	clear<I+1>(l);
}

// base case (I==tuple size, ie. one past end so do nothing)
template<std::size_t I,class... Loggers>inline
	typename std::enable_if<(I==std::tuple_size<std::tuple<Loggers...>>::value),void>::type clear(std::tuple<Loggers...>& l)
{
}


// Defines get_logger
// TODO: Clean this up (use variadic args); probably needs a helper class to get the types right without infinite recursion

template<class LA>std::tuple<typename LA::Logger> get_logger(std::tuple<LA>& t)
		{ return std::tuple<typename LA::Logger>(
					std::move(get<0>(t).get_logger())); }

template<class LA,class LB>std::tuple<typename LA::Logger,typename LB::Logger> get_logger(std::tuple<LA,LB>& t)
		{ return std::tuple<typename LA::Logger,typename LB::Logger>(
					std::move(get<0>(t).get_logger()),
					std::move(get<1>(t).get_logger())); }

template<class LA,class LB,class LC>std::tuple<typename LA::Logger,typename LB::Logger,typename LC::Logger> get_logger(std::tuple<LA,LB,LC>& t)
		{ return std::tuple<typename LA::Logger,typename LB::Logger,typename LC::Logger>(
					get<0>(t).get_logger(),
					get<1>(t).get_logger(),
					get<2>(t).get_logger()); }

template<class LA,class LB,class LC,class LD>std::tuple<typename LA::Logger,typename LB::Logger,typename LC::Logger,typename LD::Logger> get_logger(std::tuple<LA,LB,LC,LD>& t)
		{ return std::tuple<typename LA::Logger,typename LB::Logger,typename LC::Logger,typename LD::Logger>(
				std::move(get<0>(t).get_logger()),
				std::move(get<1>(t).get_logger()),
				std::move(get<2>(t).get_logger()),
				std::move(get<3>(t).get_logger())); }

template<class LA,class LB,class LC,class LD,class LE>std::tuple<typename LA::Logger,typename LB::Logger,typename LC::Logger,typename LD::Logger,typename LE::Logger> get_logger(std::tuple<LA,LB,LC,LD,LE>& t)
		{ return std::tuple<typename LA::Logger,typename LB::Logger,typename LC::Logger,typename LD::Logger,typename LE::Logger>(
				std::move(get<0>(t).get_logger()),
				std::move(get<1>(t).get_logger()),
				std::move(get<2>(t).get_logger()),
				std::move(get<3>(t).get_logger()),
				std::move(get<4>(t).get_logger())); }






#endif /* KERNELS_SOFTWARE_LOGGER_LOGGERTUPLE_HPP_ */
