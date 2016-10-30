/*
 * TIMOS.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jcassidy
 */

#include <cassert>
#include <vector>

#include "TIMOS.hpp"
#include "TIMOSReader.hpp"

#include <FullMonteSW/Geometry/Sources/Point.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>

#include <fstream>

using namespace std;

TetraMesh TIMOSReader::mesh() const
{
	TetraMeshBase tmb = mesh_base();

	if (m_useHintFileWhenAvailable && m_hintFileName.length() != 0)
		return TetraMesh(tmb,readHintFile(m_hintFileName));
	else
		return TetraMesh(tmb);
}

TetraMeshBase TIMOSReader::mesh_base() const
{
	TIMOS::Mesh tm = parse_mesh(meshFn_);

	std::vector<TetraByPointID> T(tm.T.size()+1);
	std::vector<unsigned> T_m(tm.T.size()+1);
	std::vector<Point<3,double>> P(tm.P.size()+1);

	P[0] = Point<3,double>{.0,.0,.0};
	for(unsigned i=0; i<tm.P.size(); ++i)
		P[i+1] = Point<3,double>{tm.P[i][0], tm.P[i][1], tm.P[i][2]};

	T[0] = TetraByPointID(0,0,0,0);
	for(unsigned i=0; i<tm.T.size(); ++i)
	{
		T[i+1] = tm.T[i].IDps;
		T_m[i+1] = tm.T[i].region;
	}

	TetraMeshBase M(P,T,T_m);
	return M;
}


vector<TetraMesh::FaceHint> TIMOSReader::readHintFile(const std::string fn) const
{
	std::ifstream is(fn.c_str());
	unsigned Nf=0;
	string L;

	while(is.peek() == '#' && !is.eof())
		getline(is,L);

	is >> Nf;

	vector<TetraMesh::FaceHint> H(Nf+1);

	H[0].fIDps = FaceByPointID{0U,0U,0U};
	H[0].IDts = array<unsigned,2>{0U,0U};

	for(unsigned i=1;i<=Nf && !is.eof();++i)
	{
		for(unsigned j=0;j<3;++j)
			is >> H[i].fIDps[j];
		for(unsigned j=0;j<2;++j)
			is >> H[i].IDts[j];
	}
	return H;
}

std::vector<SimpleMaterial> TIMOSReader::materials_simple() const
{
	TIMOS::Optical opt = parse_optical(optFn_);
	std::vector<SimpleMaterial> mat(opt.mat.size()+1);

	assert(opt.by_region);
	assert(!opt.matched);

	mat[0] = SimpleMaterial(0.0f,0.0f,0.0f,opt.n_ext);

	for(unsigned i=0; i<opt.mat.size(); ++i)
		mat[i+1] = SimpleMaterial(opt.mat[i].mu_s, opt.mat[i].mu_a, opt.mat[i].g, opt.mat[i].n);

	return mat;
}



Source::Abstract* TIMOSReader::sources() const
{
	assert (!sourceFn_.empty() || !"No filename specified for TIMOSReader::sources");
	std::vector<TIMOS::SourceDef> ts = parse_sources(sourceFn_);

	std::vector<Source::Abstract*> src(ts.size(),nullptr);

	boost::transform(ts,src.begin(),TIMOSReader::convertToSource);

	if (ts.size() > 1)
		return new Source::Composite(1.0,std::move(src));
	else
		return src[0];
}

std::vector<TIMOS::LegendEntry> TIMOSReader::legend() const
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

void TIMOSReader::hintFileName(const std::string fn)
{
	m_hintFileName=fn;
}

