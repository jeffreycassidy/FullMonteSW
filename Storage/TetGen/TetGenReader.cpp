#include "TetGenReader.hpp"
#include <iostream>
#include <fstream>

#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp> 

#include <FullMonteSW/Geometry/TetraMeshBase.hpp>

namespace TetGen {

TetraMeshBase TetGenReader::mesh() const
{
	vector<array<double,3>> P;
	vector<array<unsigned,4>> T_p;
	vector<unsigned> T_m;

	P = load_node_file(nodeFn());
	tie(T_p,T_m) = load_ele_file(eleFn());

	vector<Point<3,double>> Pt(P.size());
	boost::copy(P,Pt.begin());

	vector<TetraByPointID> T_pt(T_p.size());
	boost::copy(T_p,T_pt.begin());

	return TetraMeshBase(Pt,T_pt,T_m);
}

vector<array<double,3>> TetGenReader::load_node_file(const string fn) const
{
	vector<array<double,3>> P;
	ifstream is(fn);
	unsigned dim,Np,na,nb,t;

	is >> Np >> dim >> na >> nb;

	if (dim != 3 || na != 0 || nb != 0)
	{
		cerr << "Failed to read " << fn << " unexpected number of dimensions (=" << dim << "), boundary markers (=" << na << ") or attributes (=" << nb << ")" << endl;
		return P;
	}

	P.resize(Np+1);
	P[0] = array<double,3>{0,0,0};

	for(unsigned i=1;i<=Np;++i)
		is >> t >> P[i][0] >> P[i][1] >> P[i][2];

	string s;
	getline(is,s);





	return P;
}

pair<vector<array<unsigned,4>>,vector<unsigned>> TetGenReader::load_ele_file(const string fn) const
{
	ifstream is(fn);
	unsigned t,dim,Nt,nr;

	is >> Nt >> dim >> nr;

	vector<array<unsigned,4>> T_p;
	vector<unsigned> T_m;

	if (dim != 4 || (nr != 1 && nr != 0))
	{
		cerr << "Failed to read " << fn << "; unexpected number of dimensions (=" << dim << ") or region markings (=" << nr << ")" << endl;
		return make_pair(T_p,T_m);
	}

	T_m.resize(Nt+1,0);
	T_p.resize(Nt+1,array<unsigned,4>{0,0,0,0});

	for(unsigned i=1;i<=Nt;++i)
	{
		is >> t >> T_p[i][0] >> T_p[i][1] >> T_p[i][2] >> T_p[i][3];
		if (nr == 1)
			is >> T_m[i];
	}

	string s;
	getline(is,s);


	// remap material IDs
	vector<unsigned> matTetCount;

	// scan materials, mark used materials with 1
	for(unsigned i=1; i<T_m.size(); ++i)
	{
		if (T_m[i] >= matTetCount.size())
			matTetCount.resize(T_m[i]+1,0);
		matTetCount[T_m[i]]++;
	}

	// create the map
	vector<unsigned> matMap(matTetCount.size(),0);
	for(unsigned i=0,j=1; i < matTetCount.size(); ++i, j+=matTetCount[i]>0)
		matMap[i]=j;

	// remap values
	for(unsigned& t : T_m)
		t = matMap[t];

	unsigned i=1,j=0;
	for(unsigned &m : matTetCount)
	{
		cout << "  " << j++ << " (" << m << ") -> ";
		cout << (m = m ? i++ : 0) << endl;
	}

	cout << "After material mapping, there are " << i << " materials including external environment" << endl;


	return make_pair(T_p,T_m);
}

};
