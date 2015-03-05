/*
 * Test.cpp
 *
 *  Created on: Mar 4, 2015
 *      Author: jcassidy
 */

#include "TIMOSReader.hpp"

#include <string>

using namespace std;

int main(int argc,char **argv)
{
	string fnroot="/home/jcassidy/src/FullMonteSW/data/mouse";

	TIMOS::Mesh 				M 	= TIMOS::parse_mesh(fnroot+".mesh");
	std::vector<TIMOS::Source>  src = TIMOS::parse_sources(fnroot+".source");
	TIMOS::Optical 				opt = TIMOS::parse_optical(fnroot+".opt");

	cout << "Read mesh with " << M.P.size() << " points and " << M.T.size() << " tetras" << endl;

	cout << "Total " << opt.mat.size() << " materials" << endl;

	cout << "Coding by region? " << opt.by_region << endl;
	cout << "Matched boundary? " << opt.matched << endl;
	cout << "  External n=" << opt.n_ext << endl;

	return 0;
}
