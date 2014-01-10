#include <vector>
#include "newgeom.hpp"

#include "TriMesh.hpp"
#include <iostream>
#include <fstream>

using namespace std;

void TriMesh::writePoints(string fn) const
{
	if (P.size() < 2)
	{
		cerr << "No points to write" << endl;
		return;
	}
	ofstream os(fn.c_str(),ios_base::out);
	if(!os.good())
	{
		cerr << "Failed to open file" << endl;
		return;
	}
	os << P.size()-1 << endl;
	for(vector<Point<3,double> >::const_iterator it=P.begin()+1; it != P.end(); ++it)
		os << (*it)[0] << ' ' << (*it)[1] << ' ' << (*it)[2] << endl;
}

vector<int> TriMesh::matchTo(const TetraMesh& mesh)
{
	vector<int> v(F.size(),0);
	vector<int>::iterator v_it=v.begin()+1;

	for(vector<FaceByPointID>::const_iterator it = F.begin()+1; it != F.end(); ++it,++v_it)
	{
		*v_it=mesh.getFaceID(*it);
//		if (! *v_it)
//			cerr << "No match for face " << *it << endl;
	}

	vector<Point<3,double> >::const_iterator a_it=P.begin()+1;
	TetraMesh::point_const_iterator b_it=mesh.pointBegin();

	unsigned i=1,mismatch_count=0;
	for(; a_it != P.end() && b_it != mesh.pointEnd(); ++a_it,++b_it,++i)
		mismatch_count += Vector<3,double>(*a_it,*b_it).norm2_l2() > 1e-6;
	cout << "Checked " << i << " points, found " << mismatch_count << " mismatches" << endl;
	return v;
}

void TriMesh::fromVTKFile(string fn)
{
    ifstream is(fn.c_str(),ios_base::in);
	unsigned Np,Nf_el,Nf,N_data,Np_el;

	if(!is.good())
	{
		cerr << "Failed to open file " << fn << endl;
		return;
	}

	is.ignore(1000,'\n');
	is.ignore(1000,'\n');
	is.ignore(1000,'\n');
	is.ignore(1000,'\n');

    string str,ptype;

    is >> str;

    if (str == "POINTS")
    {
        is >> Np;
        is >> ptype;
        if (ptype == "float")
        {
        }
        else
            cerr << "Unexpected data type - expecting \"float\", found \"" << ptype << "\"" << endl;
    }

    P.clear();
    P.resize(Np+1);

    vector<Point<3,double> >::iterator it=P.begin();
    ++it;

    for(unsigned i=0; i<Np; ++i,++it)
        is >> *it;

    is >> str;

    if (str=="POLYGONS")
        is >> Nf >> Nf_el;
	else
	{
        cerr << "Expecting \"POLYGONS\", found \"" << str << "\" instead" << endl;
	}

    if (Nf_el != 4*Nf)
        cerr << "Error: " << Nf << " polygons, " << Nf_el << " elements (expecting 4 values each=" << ")" << endl;

    F.clear();
    F.resize(Nf+1);

    vector<FaceByPointID>::iterator f_it=F.begin();
	++f_it;

    for(unsigned i=0;i<Nf;++i,++f_it)
    {
        is >> Np_el >> *f_it;		// add one to get 1-based coords as used in program
		(*f_it)[0]++;
		(*f_it)[1]++;
		(*f_it)[2]++;
        if (Np_el != 3)
            cerr << "Error: expecting 3 elements per polygon, found " << Np_el << endl;
    }

    is >> str;

    if (str=="POINT_DATA")
        is >> N_data;
	else
		cerr << "Expecing \"POINT_DATA\", found \"" << str << "\" instead" << endl;

    is >> str;

    if (str != "SCALARS")
        cerr << "Expecting \"SCALARS\", found \"" << str << "\" instead" << endl;

    is >> str;

    is >> str;
    if (str != "float")
        cerr << "Expecting \"float\", found \"" << str << "\" instead" << endl;

    is >> str;
    if (str != "LOOKUP_TABLE")
        cerr << "Expecting \"LOOKUP_TABLE\", found \"" << str << "\" instead" << endl;

    is >> str;

    vector<double> scalars;
    scalars.resize(Np+1);       // one scalar per point
    vector<double>::iterator s_it=scalars.begin();

    ++s_it;

    for(unsigned i=0;i<Np;++i,++s_it)
        is >> *s_it;

	cout << "First 10 points: " << endl;
	for(vector<Point<3,double> >::const_iterator it=P.begin()+1; it != P.begin()+11; ++it)
		cout << *it << endl;
	cout << "Last: " << P.back() << endl;

	cout << "First 10 faces: " << endl;
	for(vector<FaceByPointID >::const_iterator it=F.begin()+1; it != F.begin()+11; ++it)
		cout << *it << endl;
	cout << "Last: " << F.back() << endl;

	cout << "First 10 scalars: " << endl;
	for(vector<double>::const_iterator it=scalars.begin()+1; it != scalars.begin()+11; ++it)
		cout << *it << endl;
	cout << "Last: " << scalars.back() << endl;
}
