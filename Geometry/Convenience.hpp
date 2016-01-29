/*
 * Convenience.hpp
 *
 *  Created on: Jan 28, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_CONVENIENCE_HPP_
#define GEOMETRY_CONVENIENCE_HPP_

#include <array>
#include <boost/range/algorithm.hpp>

template<typename U,typename T,std::size_t N>std::array<U,N> convertArrayTo(std::array<T,N> a)
{
	std::array<U,N> o;
	boost::copy(a,o.begin());
	return o;
}



#endif /* GEOMETRY_CONVENIENCE_HPP_ */
