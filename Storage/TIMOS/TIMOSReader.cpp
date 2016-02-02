/*
 * TIMOS.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jcassidy
 */

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <sstream>

#include <sys/stat.h>

#include <unordered_map>

#include "TIMOS.hpp"
#include "TIMOSReader.hpp"

#include <FullMonte/Geometry/Sources/PointSource.hpp>
#include <FullMonte/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonte/Geometry/Sources/PencilBeam.hpp>
#include <FullMonte/Geometry/Sources/Volume.hpp>
#include <FullMonte/Geometry/Sources/Composite.hpp>

using namespace std;



TetraMesh TIMOSReader::mesh() const
{
	assert (!meshFn_.empty() || !"No filename specified for TIMOSReader::mesh");
	TIMOS::Mesh tm = parse_mesh(meshFn_);

	std::vector<TetraByPointID> T(tm.T.size()+1);
	std::vector<unsigned> T_m(tm.T.size()+1);
	std::vector<array<float,3>> P(tm.P.size()+1);

	P[0] = std::array<float,3>{.0f,.0f,.0f};
	for(unsigned i=0; i<tm.P.size(); ++i)
		P[i+1] = std::array<float,3>{tm.P[i][0], tm.P[i][1], tm.P[i][2]};

	T[0] = TetraByPointID(0,0,0,0);
	for(unsigned i=0; i<tm.T.size(); ++i)
	{
		T[i+1] = tm.T[i].IDps;
		T_m[i+1] = tm.T[i].region;
	}

	TetraMesh M(P,T,T_m);
	return M;
}

std::vector<SimpleMaterial> TIMOSReader::materials_simple() const
{
	assert (!optFn_.empty() || !"No filename specified for TIMOSReader::materials");
	TIMOS::Optical opt = parse_optical(optFn_);
	std::vector<SimpleMaterial> mat(opt.mat.size()+1);

	assert(opt.by_region);
	assert(!opt.matched);

	mat[0].mu_s=0.0;
	mat[0].mu_a=0.0;
	mat[0].g=1.0;
	mat[0].n=opt.n_ext;

	for(unsigned i=0; i<opt.mat.size(); ++i)
	{
		mat[i+1].mu_s = opt.mat[i].mu_s;
		mat[i+1].mu_a = opt.mat[i].mu_a;
		mat[i+1].g = opt.mat[i].g;
		mat[i+1].n = opt.mat[i].n;
	}

	return mat;
}



Source::Base* TIMOSReader::sources() const
{
	assert (!sourceFn_.empty() || !"No filename specified for TIMOSReader::sources");
	std::vector<TIMOS::SourceDef> ts = parse_sources(sourceFn_);

	std::vector<Source::Base*> src(ts.size(),nullptr);

	boost::transform(ts,src.begin(),TIMOSReader::convertToSource);

	if (ts.size() > 1)
		return new Source::Composite(1.0,std::move(src));
	else
		return src[0];
}

std::vector<LegendEntry> TIMOSReader::legend() const
{
	std::vector<LegendEntry> l = parse_legend(legendFn_);

	std::vector<LegendEntry> L(l.size());

	for(unsigned i=0; i<L.size(); ++i)
	{
		L[i].label = l[i].label;
		L[i].colour = l[i].colour;
	}
	return L;
}


//bool TetraMeshBase::readFileMatlabTP(string fn)
//{
//	ifstream is(fn.c_str(),ios_base::in);
//
//    if(!is.good()){
//        cerr << "Failed to open " << fn << " for reading; abort" << endl;
//        return false;
//    }
//	int Nt,Np;
//
//    // read sizes
//    is >> Np;
//	is >> Nt;
//
//	// read point coordinates -- uses 1-based addressing
//	P.resize(Np+1);
//	P[0]=Point<3,double>();
//	for (vector<Point<3,double> >::iterator it = P.begin()+1; it != P.end(); ++it)
//		is >> *it;
//
//	T_p.resize(Nt+1);
//    T_m.resize(Nt+1);
//	unsigned t[4]={0,0,0,0},i=1,max_m=0;
//	T_p[0]=TetraByPointID(t);
//    T_m[0]=0;
//    TetraByPointID IDps;
//	for (vector<TetraByPointID>::iterator it=T_p.begin()+1; it != T_p.end(); ++it,++i)
//	{
//		is >> IDps;
//		is >> T_m[i];
//        *it=IDps.getSort();
//		max_m = max(max_m,T_m[i]);
//	}
//
//	return true;
//}
//
