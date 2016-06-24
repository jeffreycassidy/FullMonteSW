/*
 * TetGenBase.hpp
 *
 *  Created on: May 16, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_TETGEN_TETGENBASE_HPP_
#define STORAGE_TETGEN_TETGENBASE_HPP_

#include <string>

namespace TetGen {

class TetGenBase {

public:
	TetGenBase(){}
	TetGenBase(const std::string pfx) : nodeFn_(pfx+".node"), eleFn_(pfx+".ele"){}

	std::string nodeFn() const 				{ return nodeFn_; }
	void nodeFn(const std::string fn){ nodeFn_=fn; }

	std::string eleFn() const { return eleFn_; }
	void eleFn(const std::string fn){ eleFn_=fn; }

private:
	std::string nodeFn_;
	std::string eleFn_;
};

}



#endif /* STORAGE_TETGEN_TETGENBASE_HPP_ */
