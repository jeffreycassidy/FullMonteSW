/*
 * FloatVectorBase.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#include "FloatVectorBase.hpp"
#include <iostream>

std::ostream& operator<<(std::ostream& os,__m256 x)
{
	float tmp[8];
	_mm256_store_ps(tmp,x);

	const auto w = os.width();

	for(unsigned i=0;i<8;++i)
		std::cout << std::setw(w) << tmp[i] << ' ';
	return os;
}



