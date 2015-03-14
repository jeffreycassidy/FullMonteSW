/*
 * TIMOS.hpp
 *
 *  Created on: Mar 5, 2015
 *      Author: jcassidy
 */

#ifndef TIMOSREADER_HPP_
#define TIMOSREADER_HPP_

#include "TIMOS.hpp"

#include "TIMOSLexer.h"
#include "TIMOSParser.h"

#include <string>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>

#include <FullMonte/Storage/CommonParser/ANTLRParser.hpp>

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>

class Reader {
public:
	virtual TetraMesh 						mesh()			const=0;
	virtual std::vector<Material> 			materials() 	const=0;
	virtual std::vector<SourceDescription*> sources()		const=0;

	virtual void clear()=0;
};

class TIMOSReader {
	std::string sourceFn_,optFn_,meshFn_,legendFn_;
public:
	TIMOSReader() {}
	TIMOSReader(std::string pfx) : sourceFn_(pfx+".source"),optFn_(pfx+".opt"),meshFn_(pfx+".mesh"),legendFn_(pfx+".legend"){}

	virtual ~TIMOSReader(){}

	void setMeshFileName(std::string fn){ meshFn_=fn; }
	void setOpticalFileName(std::string fn){ optFn_=fn; }
	void setSourceFileName(std::string fn){ sourceFn_=fn; }
	void setLegendFileName(std::string fn){ legendFn_=fn; }

	virtual TetraMesh						mesh() const;
	virtual std::vector<Material>			materials() const;
	virtual std::vector<SourceDescription*>	sources() const;

	virtual std::vector<SimpleMaterial>		materials_simple() const;

	virtual std::vector<LegendEntry> 		legend() const;

	virtual void clear(){}
};

namespace TIMOS {

class ParserDef {
	static const std::unordered_map<int,std::string> toks;

public:
	MAKE_LEXER_DEF(TIMOS)
	MAKE_PARSER_DEF(TIMOS,sourcefile)

	ADD_START_RULE(TIMOS,Mat,matfile)
	ADD_START_RULE(TIMOS,Mesh,meshfile)
	ADD_START_RULE(TIMOS,Source,sourcefile)
	ADD_START_RULE(TIMOS,Legend,legendfile)

};




class sourcefile_ast_visitor : public ANTLR3CPP::ast_visitor {
	std::vector<Source> src;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		if (bt.getTokenType()==SOURCE)
		{
			assert(bt.getChildCount()==2);
			assert(bt.getChild(0).getTokenType() == INT);

			unsigned w = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());

			do_expand(bt.getChild(1));

			src.back().w = w;
		}
		else if (bt.getTokenType()==SOURCE_VOL)
		{
			assert(bt.getChildCount()==1);
			assert(bt.getChild(0).getTokenType() == INT);

			src.emplace_back();
			src.back().type = Source::Types::Volume;
			src.back().details.vol.tetID = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
		}
		else if (bt.getTokenType()==SOURCE_FACE)
		{
			assert(bt.getChildCount()==1);
			assert(bt.getChild(0).getTokenType() == TRI);
			src.emplace_back();
			src.back().type = Source::Types::Face;
			src.back().details.face.IDps = ANTLR3CPP::convert_tuple<std::array<unsigned,3>>(bt.getChild(0));
		}
		else if (bt.getTokenType()==SOURCE_PB)
		{
			assert(bt.getChildCount()==3);
			assert(bt.getChild(0).getTokenType() == INT);
			assert(bt.getChild(1).getTokenType() == POINT);
			assert(bt.getChild(2).getTokenType() == POINT);

			src.emplace_back();
			src.back().type = Source::Types::PencilBeam;
			src.back().details.pencilbeam.tetID = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			src.back().details.pencilbeam.pos = ANTLR3CPP::convert_tuple<std::array<double,3>>(bt.getChild(1));
			src.back().details.pencilbeam.dir = ANTLR3CPP::convert_tuple<std::array<double,3>>(bt.getChild(2));
		}
		else
			visit_children(bt);
	}
public:

	const std::vector<Source>& sources() const { return src; }
};

class meshfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	TIMOS::Mesh M_;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		if (bt.getTokenType()==POINT)
			M_.P.push_back(ANTLR3CPP::convert_tuple<std::array<double,3>>(bt));
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


class optfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	Optical opt_;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		unsigned tu;
		switch(bt.getTokenType())
		{
		case MATFILE:
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

public:

	const Optical& opt() const { return opt_; }
};


class legendfile_ast_visitor : public ANTLR3CPP::ast_visitor {
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


};




#endif /* TIMOS_HPP_ */
