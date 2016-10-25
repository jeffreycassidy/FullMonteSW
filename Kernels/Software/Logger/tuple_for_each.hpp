/*
 * tuple_for_each.hpp
 *
 *  Created on: Oct 19, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_TUPLE_FOR_EACH_HPP_
#define KERNELS_SOFTWARE_LOGGER_TUPLE_FOR_EACH_HPP_

#include <tuple>


/** Provides a for_each mechanism operating over tuples, with the elements from left (get<0>) to right (get<N-1>) begin
 * passed to a functor. The functor is passed and returned by value.
 *
 * FuncT tuple_for_each(std::tuple<E0,E1,...> t,FuncT f)
 *
 * @tparam I		The tuple index currently being operated on in the recursion
 * @tparam FuncT	An object with operator(e) defined for each of the elements e<i> of type E<i> in the tuple
 * @tparam Ei		i'th tuple element
 */

template<std::size_t I=0,typename FuncT,typename... Tp>inline typename std::enable_if<I==sizeof...(Tp),FuncT>::type
		tuple_for_each(const std::tuple<Tp...>&,FuncT f)
{
	return f;
}

template<std::size_t I=0,typename FuncT,typename... Tp>inline typename std::enable_if<I < sizeof...(Tp),FuncT>::type
		tuple_for_each(const std::tuple<Tp...>& t,FuncT f)
{
	return tuple_for_each<I+1,FuncT,Tp...>(t,f(std::get<I>(t)));
}


#endif /* KERNELS_SOFTWARE_LOGGER_TUPLE_FOR_EACH_HPP_ */
