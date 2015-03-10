/*
 * Test.cpp
 *
 *  Created on: Mar 4, 2015
 *      Author: jcassidy
 */

#include "TIMOSReader.hpp"

#include <string>
#include <boost/range/adaptor/indexed.hpp>

using namespace std;

int main(int argc,char **argv)
{
	string fnroot="/Users/jcassidy/src/FullMonteSW/data/mouse";

	if (argc >= 2)
		fnroot=argv[1];

	TIMOSReader R(fnroot);

	TetraMesh M = R.mesh();
	vector<SourceDescription*> src = R.sources();
	vector<Material> mat = R.materials();

	//vector<TIMOS::Region> reg = R.legend();

	vector<LegendEntry> reg = TIMOS::parse_legend(fnroot+".legend");

	cout << "Read mesh with " << M.getNp() << " points and " << M.getNt() << " tetras" << endl;
	cout << "Total " << mat.size() << " materials" << endl;
	cout << "Total " << src.size() << " source" << endl;

	for(const auto& r : reg | boost::adaptors::indexed(0U))
		cout << setw(2) << r.index() << ": " << r.value().label << "    (" << r.value().colour[0] << ',' << r.value().colour[1] << ',' << r.value().colour[2] << ')' << endl;

//	for(const TIMOS::Material& m : opt.mat)
//		cout << "  " <<

	for(const SourceDescription* s : src)
		cout << *s << endl;

	for(const Material& m : mat)
		cout << m << endl;

//	for(const TIMOS::Source& s : src)
//		cout << "  " << s << endl;
//
//	cout << "Coding by region? " << opt.by_region << endl;
//	cout << "Matched boundary? " << opt.matched << endl;
//	cout << "  External n=" << opt.n_ext << endl;

	return 0;
}
