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

// dispatches the event to all members of a tuple
template<std::size_t I=0,class LoggerTuple,class EventTag,typename... Args>inline
	typename std::enable_if<( I < std::tuple_size<LoggerTuple>::value),void>::type log_event(LoggerTuple& l,EventTag e,Args... args)
{
	log_event(get<I>(l),e,args...);
	log_event<I+1>(l,e,args...);
}

// base case (I==tuple size, ie. one past end so do nothing)
template<std::size_t I,class LoggerTuple,class EventTag,typename... Args>inline
	typename std::enable_if<(I==std::tuple_size<LoggerTuple>::value),void>::type log_event(LoggerTuple& l,EventTag e,Args... args)
{}

// Good idea from Stack Overflow

struct append_results
{
	append_results(std::list<OutputData*>& l_) : l(l_){};
	template<typename L>void operator()(const L& i) const { l.splice(l.end(), i.results()); }
private:
	std::list<OutputData*>& l;
};

template<std::size_t I=0,typename FuncT,typename... Tp>inline typename std::enable_if<I==sizeof...(Tp),void>::type
		tuple_for_each(const std::tuple<Tp...>&,FuncT){}

template<std::size_t I=0,typename FuncT,typename... Tp>inline typename std::enable_if<I < sizeof...(Tp),void>::type
		tuple_for_each(const std::tuple<Tp...>& t,FuncT f)
{
	f(std::get<I>(t));
	tuple_for_each<I+1,FuncT,Tp...>(t,f);
}


// Defines get_worker
// TODO: Clean this up (use variadic args); probably needs a helper class to get the types right without infinite recursion

template<class LA>std::tuple<typename LA::ThreadWorker> get_worker(std::tuple<LA>& t)
		{ return make_tuple(get<0>(t).get_worker()); }

template<class LA,class LB>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker> get_worker(std::tuple<LA,LB>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker()); }

template<class LA,class LB,class LC>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker,typename LC::ThreadWorker> get_worker(std::tuple<LA,LB,LC>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker(),get<2>(t).get_worker()); }

template<class LA,class LB,class LC,class LD>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker,typename LC::ThreadWorker,typename LD::ThreadWorker> get_worker(std::tuple<LA,LB,LC,LD>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker(),get<2>(t).get_worker(),get<3>(t).get_worker()); }

template<class LA,class LB,class LC,class LD,class LE>std::tuple<typename LA::ThreadWorker,typename LB::ThreadWorker,typename LC::ThreadWorker,typename LD::ThreadWorker,typename LE::ThreadWorker> get_worker(std::tuple<LA,LB,LC,LD,LE>& t)
		{ return make_tuple(get<0>(t).get_worker(),get<1>(t).get_worker(),get<2>(t).get_worker(),get<3>(t).get_worker(),get<4>(t).get_worker()); }






#endif /* KERNELS_SOFTWARE_LOGGER_LOGGERTUPLE_HPP_ */
