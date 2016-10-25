/*
 * Print.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_PRINT_HPP_
#define GEOMETRY_SOURCES_PRINT_HPP_

#include "Abstract.hpp"
#include <iostream>

namespace Source
{

std::ostream& operator<<(std::ostream& os,const Abstract& b);

}




#endif /* GEOMETRY_SOURCES_PRINT_HPP_ */
