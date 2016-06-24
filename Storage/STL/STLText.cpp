/*
 * STLText.cpp
 *
 *  Created on: May 16, 2015
 *      Author: jcassidy
 */

#include "STLText.hpp"
#include <iostream>
#include <fstream>

using namespace std;

namespace STL {
namespace Text {

/** writeSTLText gives a standard STL ascii output file
 *
 * Note: drops the first point and first triangle because FullMonte conventionally uses a null first point and first triangle
 * which helps it play nice with Matlab's 1-based indexing (indices don't change, what changes is whether or not the first list
 * element is saved/printed/etc)
 *
 */

void writeSTLText(const std::string fn,
		const std::vector<std::array<float,3>>& P,
		const std::vector<std::array<unsigned,3>>& T,
		const std::string name)
{
	ofstream os(fn.c_str());
	os << "solid " << name << endl;
	for(unsigned i=1;i<T.size();++i)
	{
		os << "  facet normal 0 0 0" << endl;
		os << "    outer loop" << endl;
		for(unsigned j=0;j<3;++j)
		{
			os << "      vertex";
			for(unsigned k=0;k<3;++k)
				os << ' ' << P[T[i][j]][k];
			os << endl;
		}
		os << "    endloop" << endl;
		os << "  endfacet" << endl;
	}
	os << "endsolid " << name << endl;
}

}
}
