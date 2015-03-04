/*
 * Test.cpp
 *
 *  Created on: Mar 4, 2015
 *      Author: jcassidy
 */

#include "../../SourceDescription.hpp"
#include "../../Material.hpp"

#include "../CommonParser/ANTLRParser.hpp"

#include "TIMOSLexer.h"
#include "TIMOSParser.h"
#include <antlr3.h>

#define MAKE_LEXER_DEF(pfx) struct Lexer { \
	typedef p##pfx##Lexer ptr_t; \
	static ptr_t CreateLexer(pANTLR3_INPUT_STREAM i){ return pfx##LexerNew(i); } \
};

#define MAKE_PARSER_DEF(pfx,default_start_rule) struct Parser { \
	typedef p##pfx##Parser ptr_t; \
	typedef pfx##Parser_sourcefile_return startrule_return_t; \
	static ptr_t CreateParser(pANTLR3_COMMON_TOKEN_STREAM i){ return pfx##ParserNew(i); } \
	typedef startrule_return_t(*startrule_t)(ptr_t); \
	static startrule_t startrule(ptr_t p){ return p->default_start_rule; } \
};

#define ADD_START_RULE(pfx,tag,rulename) struct tag { \
	typedef pfx##Parser_##rulename##_return startrule_return_t; \
	typedef startrule_return_t(*startrule_t)(Parser::ptr_t); \
	static  startrule_t startrule(Parser::ptr_t p){ return p->rulename; } \
};

struct ANTLR_TIMOS {
	MAKE_LEXER_DEF(TIMOS)
	MAKE_PARSER_DEF(TIMOS,sourcefile)

	ADD_START_RULE(TIMOS,Mat,matfile)
	ADD_START_RULE(TIMOS,Mesh,meshfile)
	ADD_START_RULE(TIMOS,Source,sourcefile)
};


#include <string>

using namespace std;

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
	vector<array<float,3>> P;
	vector<array<unsigned,4>> T;

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
	const vector<array<float,3>> points() const { return P; }
	const vector<array<unsigned,4>> tetras() const { return T; }
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



int main(int argc,char **argv)
{
	string fnroot="/Users/jcassidy/src/FullMonteSW/data/mouse";

	ANTLRParser<ANTLR_TIMOS> P(fnroot + ".source");

	ANTLR3CPP::base_tree bt = P.parse();

	sourcefile_ast_visitor SV;

	SV.walk(bt);

	ANTLRParser<ANTLR_TIMOS> M(fnroot + ".mesh");
	ANTLR3CPP::base_tree mbt = M.parse<ANTLR_TIMOS::Mesh>();

	meshfile_ast_visitor MV;

	MV.walk(mbt);


	cout << "Read mesh with " << MV.points().size() << " points and " << MV.tetras().size() << " tetras" << endl;

	ANTLRParser<ANTLR_TIMOS> O(fnroot + ".opt");
	optfile_ast_visitor OV;

	OV.walk(O.parse<ANTLR_TIMOS::Mat>());

	for(const auto & m : OV.materials())
		cout << m << endl;

	cout << "Total " << OV.materials().size() << " materials" << endl;

	cout << "Coding by region? " << OV.per_region() << endl;
	cout << "Matched boundary? " << OV.matched() << endl;
	cout << "  External n=" << OV.n_ext() << endl;

	return 0;
}
