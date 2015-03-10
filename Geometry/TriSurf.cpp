#include <iostream>
#include <fstream>
#include "TriSurf.hpp"

using namespace std;

void TriSurf::writeASCII(const string& fn) const
{
	ofstream os(fn.c_str());

	os << P.size() << endl << F.size() << endl;

	for(const Point<3,double>& p : P)
		os << p[0] << ' ' << p[1] << ' ' << p[2] << endl;

	for(const FaceByPointID& IDfs : F)
		os << IDfs[0] << ' ' << IDfs[1] << ' ' << IDfs[2] << endl;
}
