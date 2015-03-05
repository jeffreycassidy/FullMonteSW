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

#include "../CommonParser/ANTLRParser.hpp"

namespace TIMOS {

class ParserDef {
	static const std::unordered_map<int,std::string> toks;

public:
	MAKE_LEXER_DEF(TIMOS)
	MAKE_PARSER_DEF(TIMOS,sourcefile)

	ADD_START_RULE(TIMOS,Mat,matfile)
	ADD_START_RULE(TIMOS,Mesh,meshfile)
	ADD_START_RULE(TIMOS,Source,sourcefile)

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


};




#endif /* TIMOS_HPP_ */
