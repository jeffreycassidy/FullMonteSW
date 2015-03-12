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

#include "TIMOSReader.hpp"

using namespace std;

const std::unordered_map<int,string> TIMOS::ParserDef::toks{
#include "TIMOS_tokens.h"
};


TetraMesh TIMOSReader::mesh() const
{
	assert (!meshFn_.empty() || !"No filename specified for TIMOSReader::mesh");
	TIMOS::Mesh tm = TIMOS::parse_mesh(meshFn_);

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

	TetraMesh M(P,T,T_m);
	return M;
}

std::vector<Material> TIMOSReader::materials() const
{
	assert (!optFn_.empty() || !"No filename specified for TIMOSReader::materials");
	TIMOS::Optical opt = TIMOS::parse_optical(optFn_);
	std::vector<Material> mat(opt.mat.size()+1);

	assert(opt.by_region);
	assert(!opt.matched);

	mat[0] = Material(0,0,0,1.0);
	for(unsigned i=0; i<opt.mat.size(); ++i)
		mat[i+1] = Material(opt.mat[i].mu_a,opt.mat[i].mu_s,opt.mat[i].g,opt.mat[i].n);

	return mat;
}

std::vector<SourceDescription*> TIMOSReader::sources() const
{
	assert (!sourceFn_.empty() || !"No filename specified for TIMOSReader::sources");
	std::vector<TIMOS::Source> ts = TIMOS::parse_sources(sourceFn_);
	std::vector<SourceDescription*> src(ts.size(),nullptr);

	for(unsigned i=0;i<ts.size();++i)
	{
		switch(ts[i].type)
		{
		case TIMOS::Source::Types::Volume:
			src[i] = new VolumeSourceDescription(ts[i].details.vol.tetID,ts[i].w);
			break;

		case TIMOS::Source::Types::PencilBeam:
			src[i] = new PencilBeamSourceDescription(
					Point<3,double>(ts[i].details.pencilbeam.pos.data()),
					UnitVector<3,double>(ts[i].details.pencilbeam.dir.data()),
					ts[i].w,
					ts[i].details.pencilbeam.tetID);
			break;

		case TIMOS::Source::Types::Face:
			src[i] = new FaceSourceDescription(
					FaceByPointID(ts[i].details.face.IDps),
					ts[i].w);
			break;

		case TIMOS::Source::Types::Point:
			src[i] = new IsotropicPointSourceDescription(
					Point<3,double>(ts[i].details.point.pos.data()),
					ts[i].w);
			break;

		default:
			cerr << "Unsupported source type #" << ts[i].type << endl;
		}
	}

	return src;
}

std::vector<LegendEntry> TIMOSReader::legend() const
{
	std::vector<TIMOS::LegendEntry> l = TIMOS::parse_legend(legendFn_);

	std::vector<LegendEntry> L(l.size());

	for(unsigned i=0; i<L.size(); ++i)
	{
		L[i].label = l[i].label;
		L[i].colour = l[i].colour;
	}
	return L;
}

namespace TIMOS {

std::vector<Source>	parse_sources(std::string fn)
{
	ANTLRParser<ParserDef> P(fn);
	ANTLR3CPP::base_tree bt = P.parse();

	TIMOS::sourcefile_ast_visitor SV;

	SV.walk(bt);
	return SV.sources();
}

Mesh parse_mesh(std::string fn)
{
	ANTLRParser<ParserDef> M(fn);
	ANTLR3CPP::base_tree mbt = M.parse<TIMOS::ParserDef::Mesh>();

	TIMOS::meshfile_ast_visitor MV;

	MV.walk(mbt);
	return MV.mesh();
}

Optical parse_optical(std::string fn)
{
	ANTLRParser<ParserDef> O(fn);
	TIMOS::optfile_ast_visitor OV;

	OV.walk(O.parse<ParserDef::Mat>());
	return OV.opt();
}

std::vector<LegendEntry> parse_legend(std::string fn)
{
	struct stat info;
	if ( stat(fn.c_str(),&info) != 0 )
		return std::vector<LegendEntry>();

	ANTLRParser<ParserDef> R(fn);
	TIMOS::legendfile_ast_visitor RV;

	RV.walk(R.parse<ParserDef::Legend>());
	return RV.legend();
}


ostream& operator<<(ostream& os,const Source& s)
{
	switch(s.type){
	case Source::Types::PencilBeam:
		os << "Pencil beam at (" <<
		s.details.pencilbeam.pos[0] << ',' << s.details.pencilbeam.pos[1] << ',' << s.details.pencilbeam.pos[2] << ")" <<
		" directed at (" <<
		s.details.pencilbeam.dir[0] << ',' << s.details.pencilbeam.dir[1] << ',' << s.details.pencilbeam.dir[2] << ")" <<
		" tetra ID=" << s.details.pencilbeam.tetID;
		break;

	case Source::Types::Face:
		os << "Face source at face [" << s.details.face.IDps[0] << ',' << s.details.face.IDps[1] << ',' << s.details.face.IDps[2] << ']';
		break;

	case Source::Types::Volume:
		os << "Volume source (" << s.type << ") in tetra " << s.details.vol.tetID;
		break;

	case Source::Types::Point:
		os << "Point source at (" << s.details.point.pos[0] << ',' << s.details.point.pos[1] << ',' << s.details.point.pos[2] << ')';

	default:
		os << "(unknown type - " << s.type << ')';
	}
	return os << " weight=" << s.w;
}

}
