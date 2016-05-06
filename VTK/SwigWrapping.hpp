/*
 * SwigWrapping.hpp
 *
 *  Created on: Feb 19, 2016
 *      Author: jcassidy
 */

#ifndef VTK_SWIGWRAPPING_HPP_
#define VTK_SWIGWRAPPING_HPP_

#include <utility>

struct SwigPointerInfo {
	void* 								p=nullptr;
	std::pair<const char*,const char*>	type=std::make_pair(nullptr,nullptr);
};

SwigPointerInfo readSwigPointer(const char *s);

#endif /* VTK_SWIGWRAPPING_HPP_ */
