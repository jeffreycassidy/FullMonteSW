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

#include <FullMonte/Geometry/Sources/Point.hpp>
#include <FullMonte/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonte/Geometry/Sources/PencilBeam.hpp>
#include <FullMonte/Geometry/Sources/Volume.hpp>

using namespace std;



TetraMesh TIMOSReader::mesh()
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



std::vector<Material> TIMOSReader::materials()
{
	assert (!optFn_.empty() || !"No filename specified for TIMOSReader::materials");
	TIMOS::Optical opt = parse_optical(optFn_);
	std::vector<Material> mat(opt.mat.size()+1);

	assert(opt.by_region);
	assert(!opt.matched);

	mat[0] = Material(0,0,0,1.0);
	for(unsigned i=0; i<opt.mat.size(); ++i)
		mat[i+1] = Material(opt.mat[i].mu_a,opt.mat[i].mu_s,opt.mat[i].g,opt.mat[i].n);

	return mat;
}

std::vector<SimpleMaterial> TIMOSReader::materials_simple()
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



std::vector<Source::Base*> TIMOSReader::sources()
{
	assert (!sourceFn_.empty() || !"No filename specified for TIMOSReader::sources");
	std::vector<TIMOS::SourceDef> ts = parse_sources(sourceFn_);

	std::vector<Source::Base*> src(ts.size(),nullptr);

	boost::transform(ts,src.begin(),TIMOSReader::convertToSource);

	return src;
}

std::vector<LegendEntry> TIMOSReader::legend()
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



ostream& operator<<(ostream& os,const TIMOS::SourceDef& s)
{
	switch(s.type){
	case TIMOS::SourceDef::Types::PencilBeam:
		os << "Pencil beam at (" <<
		s.details.pencilbeam.pos[0] << ',' << s.details.pencilbeam.pos[1] << ',' << s.details.pencilbeam.pos[2] << ")" <<
		" directed at (" <<
		s.details.pencilbeam.dir[0] << ',' << s.details.pencilbeam.dir[1] << ',' << s.details.pencilbeam.dir[2] << ")" <<
		" tetra ID=" << s.details.pencilbeam.tetID;
		break;

	case TIMOS::SourceDef::Types::Face:
		os << "Face source at face [" << s.details.face.IDps[0] << ',' << s.details.face.IDps[1] << ',' << s.details.face.IDps[2] << ']';
		break;

	case TIMOS::SourceDef::Types::Volume:
		os << "Volume source (" << s.type << ") in tetra " << s.details.vol.tetID;
		break;

	case TIMOS::SourceDef::Types::Point:
		os << "Point source at (" << s.details.point.pos[0] << ',' << s.details.point.pos[1] << ',' << s.details.point.pos[2] << ')';

	default:
		os << "(unknown type - " << s.type << ')';
	}
	return os << " weight=" << s.w;
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
