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

#include <unordered_map>

#include "TIMOSReader.hpp"


using namespace std;

const std::unordered_map<int,string> TIMOS::ParserDef::toks{
#include "TIMOS_tokens.h"
};

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
