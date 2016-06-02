/*
 * TIMOSParser.cpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */


#include "TIMOSAntlrParser.hpp"

#include "TIMOSLexer.h"
#include "TIMOSParser.h"

#include <FullMonteSW/Storage/CommonParser/ANTLRParser.hpp>

#include <unordered_map>


class TIMOSAntlrParser::ANTLR3LP
{
public:
	MAKE_LEXER_DEF(TIMOS)
	MAKE_PARSER_DEF(TIMOS,sourcefile)

	ADD_START_RULE(TIMOS,Mat,matfile)
	ADD_START_RULE(TIMOS,Mesh,meshfile)
	ADD_START_RULE(TIMOS,Source,sourcefile)
	ADD_START_RULE(TIMOS,Legend,legendfile)

	std::unordered_map<int,string> s_tokens;
};

const std::unordered_map<int,string> TIMOSAntlrParser::s_tokens
{
	#include "TIMOS_tokens.h"
};

class TIMOSAntlrParser::sourcefile_ast_visitor : public ANTLR3CPP::ast_visitor {
	std::vector<TIMOS::SourceDef> src;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		if (bt.getTokenType()==SOURCE)
		{
			assert(bt.getChildCount()==2);
			assert(bt.getChild(0).getTokenType() == INT);

			unsigned w = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());

			// expand the child node first
			do_expand(bt.getChild(1));

			// then finish up by setting weight
			src.back().w = w;
		}
		else if (bt.getTokenType()==SOURCE_POINT)
		{
			assert(bt.getChildCount()==1);
			assert(bt.getChild(0).getTokenType() == POINT);
			src.emplace_back();
			src.back().type = TIMOS::SourceDef::Types::Point;
			src.back().details.point.pos = ANTLR3CPP::convert_tuple<std::array<float,3>>(bt.getChild(0));
		}
		else if (bt.getTokenType()==SOURCE_VOL)
		{
			assert(bt.getChildCount()==1);
			assert(bt.getChild(0).getTokenType() == INT);

			src.emplace_back();
			src.back().type = TIMOS::SourceDef::Types::Volume;
			src.back().details.vol.tetID = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
		}
		else if (bt.getTokenType()==SOURCE_FACE)
		{
			assert(bt.getChildCount()==1);
			assert(bt.getChild(0).getTokenType() == TRI);
			src.emplace_back();
			src.back().type = TIMOS::SourceDef::Types::Face;
			src.back().details.face.IDps = ANTLR3CPP::convert_tuple<std::array<unsigned,3>>(bt.getChild(0));
		}
		else if (bt.getTokenType()==SOURCE_PB)
		{
			assert(bt.getChildCount()==3);
			assert(bt.getChild(0).getTokenType() == INT);
			assert(bt.getChild(1).getTokenType() == POINT);
			assert(bt.getChild(2).getTokenType() == POINT);

			src.emplace_back();
			src.back().type = TIMOS::SourceDef::Types::PencilBeam;
			src.back().details.pencilbeam.tetID = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			src.back().details.pencilbeam.pos = ANTLR3CPP::convert_tuple<std::array<float,3>>(bt.getChild(1));
			src.back().details.pencilbeam.dir = ANTLR3CPP::convert_tuple<std::array<float,3>>(bt.getChild(2));
		}
		else
			visit_children(bt);
	}
public:

	const std::vector<TIMOS::SourceDef>& sources() const { return src; }
};

class TIMOSAntlrParser::optfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	Optical opt_;

public:
	Optical opt() const
	{
		return opt_;
	}

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		unsigned tu;
		switch(bt.getTokenType())
		{
		case		MATFILE:
			tu = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			assert(tu == 1 || tu == 2);
			opt_.by_region = (tu==1);
			visit_children(bt);
			break;

		case EXTERIOR:
			tu = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			assert(tu == 1 || tu == 2);
			if ((opt_.matched = (tu==2)))
				assert(bt.getChildCount()==1);
			else
			{
				assert(bt.getChildCount()==2);
				opt_.n_ext = ANTLR3CPP::convert_string<float>(bt.getChild(1).getTokenText());
			}
			break;

		case MATERIAL:
			opt_.mat.emplace_back();
			std::tie(opt_.mat.back().mu_a,opt_.mat.back().mu_s,opt_.mat.back().g,opt_.mat.back().n)
			= ANTLR3CPP::convert_tuple<std::tuple<float,float,float,float>>(bt);
			break;

		default:
			visit_children(bt);
		}
	}

};

class TIMOSAntlrParser::meshfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	TIMOS::Mesh M_;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		if (bt.getTokenType()==POINT)
			M_.P.push_back(ANTLR3CPP::convert_tuple<std::array<float,3>>(bt));
		else if (bt.getTokenType()==TETRA)
		{
			assert(bt.getChildCount()==2);
			M_.T.emplace_back(
					ANTLR3CPP::convert_tuple<std::array<unsigned,4>>(bt.getChild(0)),
					ANTLR3CPP::convert_string<unsigned>(bt.getChild(1).getTokenText()));
		}
		else
			visit_children(bt);
	}

public:

	const TIMOS::Mesh& mesh() const { return M_; }
};



class legendfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	std::vector<TIMOS::LegendEntry> legend_;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		string s;
		switch(bt.getTokenType())
		{
		case LEGEND:
			legend_.reserve(bt.getChildCount());
			visit_children(bt);
			break;

		case LEGENDENTRY:
			assert(bt.getChildCount()==2);
			assert(bt.getChild(0).getTokenType()==QUOTEDSTRING);
			assert(bt.getChild(1).getTokenType()==RGBCOLOUR);

			legend_.emplace_back();
			s = ANTLR3CPP::convert_string<std::string>(bt.getChild(0).getTokenText());
			legend_.back().label  = s.substr(1,s.size()-2);
			legend_.back().colour = ANTLR3CPP::convert_tuple<std::array<float,3>>(bt.getChild(1));
			break;

		default:
			assert(/* Invalid AST in legendfile_ast_visitor*/ 0);
		}
	}

public:

	const std::vector<TIMOS::LegendEntry>& legend() const { return legend_; }
};



class TIMOSAntlrParser::legendfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	std::vector<LegendEntry> legend_;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		string s;
		switch(bt.getTokenType())
		{
		case LEGEND:
			legend_.reserve(bt.getChildCount());
			visit_children(bt);
			break;

		case LEGENDENTRY:
			assert(bt.getChildCount()==2);
			assert(bt.getChild(0).getTokenType()==QUOTEDSTRING);
			assert(bt.getChild(1).getTokenType()==RGBCOLOUR);

			legend_.emplace_back();
			s = ANTLR3CPP::convert_string<std::string>(bt.getChild(0).getTokenText());
			legend_.back().label  = s.substr(1,s.size()-2);
			legend_.back().colour = ANTLR3CPP::convert_tuple<std::array<float,3>>(bt.getChild(1));
			break;

		default:
			assert(/* Invalid AST in legendfile_ast_visitor*/ 0);
		}
	}

public:

	const std::vector<LegendEntry>& legend() const { return legend_; }
};



std::vector<TIMOS::SourceDef>	TIMOSAntlrParser::parse_sources(std::string fn) const
{
	ANTLRParser<ANTLR3LP> P(fn);
	ANTLR3CPP::base_tree bt = P.parse();

	TIMOSAntlrParser::sourcefile_ast_visitor SV;

	SV.walk(bt);
	return SV.sources();
}

TIMOS::Mesh TIMOSAntlrParser::parse_mesh(std::string fn) const
{
	ANTLRParser<ANTLR3LP> M(fn);
	ANTLR3CPP::base_tree mbt = M.parse<ANTLR3LP::Mesh>();

	TIMOSAntlrParser::meshfile_ast_visitor MV;

	MV.walk(mbt);
	return MV.mesh();
}

TIMOS::Optical TIMOSAntlrParser::parse_optical(std::string fn) const
{
	ANTLRParser<ANTLR3LP> O(fn);
	TIMOSAntlrParser::optfile_ast_visitor OV;

	OV.walk(O.parse<ANTLR3LP::Mat>());
	return OV.opt();
}

std::vector<TIMOS::LegendEntry> TIMOSAntlrParser::parse_legend(std::string fn) const
{
	struct stat info;
	if ( stat(fn.c_str(),&info) != 0 )
		return std::vector<LegendEntry>();

	ANTLRParser<ANTLR3LP> R(fn);
	TIMOSAntlrParser::legendfile_ast_visitor RV;

	RV.walk(R.parse<ANTLR3LP::Legend>());
	return RV.legend();
}



