/*
 * TIMOS.hpp
 *
 *  Created on: Mar 5, 2015
 *      Author: jcassidy
 */

#ifndef TIMOS_HPP_
#define TIMOS_HPP_

#include "TIMOSLexer.h"
#include "TIMOSParser.h"

#include "../CommonParser/ANTLRParser.hpp"

#include "../../SourceDescription.hpp"
#include "../../Material.hpp"

namespace TIMOS {

class ParserDef {
	static const std::unordered_map<int,string> toks;

public:
	MAKE_LEXER_DEF(TIMOS)
	MAKE_PARSER_DEF(TIMOS,sourcefile)

	ADD_START_RULE(TIMOS,Mat,matfile)
	ADD_START_RULE(TIMOS,Mesh,meshfile)
	ADD_START_RULE(TIMOS,Source,sourcefile)

};


class sourcefile_ast_visitor : public ANTLR3CPP::ast_visitor {
	vector<SourceDescription*> src;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		if (bt.getTokenType()==SOURCE)
		{
			assert(bt.getChildCount()==2);
			assert(bt.getChild(0).getTokenType() == INT);

			unsigned w = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());

			do_expand(bt.getChild(1));
			src.back()->setPower(w);
			cout << *src.back() << endl;
		}
		else if (bt.getTokenType()==SOURCE_VOL)
		{
			assert(bt.getChildCount()==1);
			assert(bt.getChild(0).getTokenType() == INT);

			unsigned tet = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			src.push_back(new VolumeSourceDescription(tet));
		}
		else
			visit_children(bt);
	}
};

class meshfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	std::vector<std::array<float,3>> 		P;
	std::vector<std::array<unsigned,4>> 	T;

protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		if (bt.getTokenType()==POINT)
			P.push_back(ANTLR3CPP::convert_tuple<array<float,3>>(bt));
		else if (bt.getTokenType()==TETRA)
			T.push_back(ANTLR3CPP::convert_tuple<array<unsigned,4>>(bt));
		else
			visit_children(bt);
	}

public:
	const std::vector<std::array<float,3>> points() 	const { return P; }
	const std::vector<std::array<unsigned,4>> tetras() 	const { return T; }
};


class optfile_ast_visitor : public ANTLR3CPP::ast_visitor {
	vector<Material> mat_;
	bool per_region_=true;
	bool matched_=true;
	float n_ext_=NAN;


protected:
	virtual void do_expand(ANTLR3CPP::base_tree bt)
	{
		array<float,4> a;
		unsigned by_region,m;
		switch(bt.getTokenType())
		{
		case MATFILE:
			by_region = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			assert(by_region == 1 || by_region == 2);

			visit_children(bt);
		break;

		case EXTERIOR:
			m = ANTLR3CPP::convert_string<unsigned>(bt.getChild(0).getTokenText());
			assert(m == 1 || m == 2);
			matched_ = m==1;
			if (!matched_)
				n_ext_ = ANTLR3CPP::convert_string<float>(bt.getChild(1).getTokenText());
		break;

		case MATERIAL:
			a = ANTLR3CPP::convert_tuple<array<float,4>>(bt);		// mu_a,mu_s,g,n
			mat_.emplace_back(a[0],a[1],a[2],a[3]);
		break;

		default:
			visit_children(bt);
		}
	}

public:
	bool matched() const { return matched_; }
	float n_ext() const { return n_ext_; }
	bool per_region() const { return per_region_; }
	const vector<Material> materials() const { return mat_; }
};

};




#endif /* TIMOS_HPP_ */
