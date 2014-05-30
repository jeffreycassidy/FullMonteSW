#include "PinnacleROI.hpp"
#include <iostream>
#include <iomanip>
#include <iterator>
#include <fstream>

#include "../../ece297/XMLWriter/XMLWriter.hpp"


using namespace std;

template<typename T,unsigned long D>ostream& operator<<(ostream& os,array<T,D>a)
{
	os << a[0];
	for(unsigned i=1;i<D;++i)
		os << ' ' << a[i];
	return os;
}

namespace Pinnacle {

void ROI::printDetails() const
{
	unsigned Ncurves=getprop<unsigned>("num_curve");
	const string& roiname=getprop<string>("name");
	cout << "ROI with " << curves.size() << " curves: " << roiname << " ";

	if (Ncurves != curves.size())
		cout << " Mismatch: num_curves=" << Ncurves << endl;

	else
		cout << " (OK)" << endl;

	unsigned i=0;
	for(const Curve& c : curves)
	{
		cout << "    [" << setw(3) << i++ << "] ";
		c.printDetails();
	}
}

/** Returns true if the ROI is valid.
 *
 * @param exc_ Enable exceptions (defaults to false)
 *
 * @return Valid flag (true=good)
 */

bool ROI::validate(bool exc_) const
{
	unsigned Ncurves=getprop<unsigned>("num_curve");

	if(Ncurves != curves.size())
		return false;

	return true;
}

void ROI::exportCurves(string fn_) const
{
	ofstream os(fn_.c_str());

	os << "# Output from PinnacleROI.cpp" << endl;
	os << "# First line is number of curves" << endl;
	os << "# Second line is space-separated number of points in each curve" << endl;
	os << "# Remainder are the points themselves" << endl;

	os << curves.size() << endl;

	auto Nps = make_transform_adaptor(curves,mem_fn(&Curve::getNPoints));
	copy(begin(Nps),end(Nps),ostream_iterator<unsigned>(os," "));
	os << endl;

	auto r = make_iiterator_adaptor(curves,mem_fn(&Curve::getPoints));
	/*auto osi = ostream_iterator<array<double,3>>(os,"\n");
	copy(r.first,r.second,osi);*/

	for(const array<double,3>& p : r)
		os << p << endl;
}

void ROI::exportCurvesTriangle(string fn_,const list<unsigned>& curveIDs_) const
{
	ofstream os(fn_.c_str());

	unsigned Np=0;
	for(unsigned IDc : curveIDs_)
	{
		list<Curve>::const_iterator it;
		unsigned j=0;
		for(it=curves.begin(); it != curves.end() && j < IDc; ++j,++it){}

		Np += it->getNPoints();
	}

	// write header: #verts, #dim (2), #attr, #bndmarker
	// #bnd must be 0 or 1
	os << Np << " 2 0 0" << endl;



	unsigned i=1;
	for(unsigned IDc : curveIDs_)
	{
		list<Curve>::const_iterator it;
		unsigned j=0;
		for(it=curves.begin(); it != curves.end() && j < IDc; ++j,++it){}

		for (const array<double,3>& p : it->getPoints())
			os << i++ << ' ' << p[0] << ' ' << p[1] << endl;
	}

	// #segs copies of <seg#> <endpoint> <endpoint> [bndmarker]
	// no boundary markers here

	// segments header: #segs, #bndmarker
	os << Np << " 0" << endl;

	i=1;
	for(unsigned IDc : curveIDs_)
	{
		list<Curve>::const_iterator it;
		unsigned j=0;
		for(it=curves.begin(); it != curves.end() && j < IDc; ++j,++it){}

		unsigned start=i;
		for(unsigned j=0; j<it->getNPoints()-1; ++j,++i)
			os << i << ' ' << i << ' ' << i+1 << endl;
		os << i << ' ' << start << ' ' << i << endl;
		++i;
	}

	// #holes
	// #holes copies of <hole#> <x> <y>
	os << "0" << endl;
}

}
