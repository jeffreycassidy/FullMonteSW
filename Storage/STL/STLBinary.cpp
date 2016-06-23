/*
 * STLBinary.cpp
 *
 *  Created on: May 16, 2015
 *      Author: jcassidy
 */

#include "STLBinary.hpp"

#include <fstream>
#include <iostream>

#include <unordered_map>

#include <boost/functional/hash.hpp>

using namespace std;

namespace STL {
namespace Binary {

/** loadSTLBinary loads an STL Binary file, returning a vector of points and a vector of triangles consisting of point index arrays
 * Per FullMonte convention, the first point and first triangle are nulls (0,0,0)
 *
 * STL gives triangles as 3-tuples of points; this transforms that into the usual format using a std::unordered_map into which
 * points are inserted in order of occurrence.
 *
 * This is a very simple loader which does not handle region assignment or multiple STL files
 */

pair<vector<array<float,3>>,vector<array<unsigned,3>>> loadSTLBinary(const std::string fn)
{
	ifstream is(fn.c_str());

	// get file size
	is.seekg(0,ios_base::end);
	size_t Nb = is.tellg();
	is.seekg(0,ios_base::beg);

	// read header
	Header h;
	is.read((char*)&h,sizeof(Header));

	// read data points
	vector<Entry> e(h.Ntri);

	is.read((char*)e.data(),sizeof(Entry)*h.Ntri);
	size_t Nbr = is.gcount();

	if (Nbr != h.Ntri*sizeof(Entry))
		cerr << "Read size error" << endl;

	// map points to unique vector
	unordered_map<array<float,3>,unsigned,boost::hash<array<float,3>>> m;

	m.insert(make_pair(array<float,3>{0,0,0},0));

	vector<array<float,3>> 		P;
	vector<array<unsigned,3>>	T;

	P.push_back(array<float,3>{0,0,0});
	T.push_back(array<unsigned,3>{0,0,0});

	for(const auto& tri : e)
	{
		array<unsigned,3> IDp;
		for(unsigned i=0;i<3;++i)
		{
			pair<unordered_map<array<float,3>,unsigned,boost::hash<array<float,3>>>::const_iterator,bool> p
				= m.insert(make_pair(tri.p[i],m.size()));

			if (p.second)
				P.push_back(tri.p[i]);
			IDp[i] = p.first->second;
		}
		T.push_back(IDp);
	}

	// chatty output
	cout << "Read " << T.size() << " triangles with " << P.size() << " distinct points" << endl;
	cout << "Expecting " << h.Ntri << " triangles " << endl;
	cout << "  Header '" << string((const char*)h.comm,80) << "'" << endl;
	cout << "  Byte count: " << Nb << " expecting " << (sizeof(Header)+h.Ntri*sizeof(Entry)) << endl;

	return make_pair(P,T);
}


}
}
