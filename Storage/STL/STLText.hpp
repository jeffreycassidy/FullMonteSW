/*
 * STLText.hpp
 *
 *  Created on: May 16, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_STL_STLTEXT_HPP_
#define STORAGE_STL_STLTEXT_HPP_

#include <string>
#include <vector>
#include <array>

namespace STL {
namespace Text {

// to be implemented
// void std::pair<std::vector<std::array<float,3>>,std::vector<std::array<unsigned,3>>> readSTLText

void writeSTLText(std::string fn,
		const std::vector<std::array<float,3> >& P,
		const std::vector<std::array<unsigned,3> >& T,
		std::string name="");

}
}


#endif /* STORAGE_STL_STLTEXT_HPP_ */
