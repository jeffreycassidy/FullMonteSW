/*
 * TIMOS.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jcassidy
 */

#include "TIMOSLexer.h"
#include "TIMOSParser.h"

#include "../CommonParser/ANTLRParser.hpp"

#include <antlr3.h>
#include <antlr3defs.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <sstream>

#include <unordered_map>


using namespace std;

std::unordered_map<int,string> toks{
#include "TIMOS_tokens.h"
};


#define PARSER_START(clroot,rule) clroot##Parser_##rule##_return parse_##rule(){ return parser_->rule(parser_); }

class TIMOSParserClass {
	pANTLR3_INPUT_STREAM input_=nullptr;
	pTIMOSLexer lex_=nullptr;
	pANTLR3_COMMON_TOKEN_STREAM tokens_=nullptr;
	pTIMOSParser parser_=nullptr;

public:

	TIMOSParserClass(string fn)
	{
		input_ = antlr3FileStreamNew((pANTLR3_UINT8)fn.c_str(),ANTLR3_ENC_8BIT);
		lex_ = TIMOSLexerNew(input_);
		tokens_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
				TOKENSOURCE(lex_));
		parser_ = TIMOSParserNew(tokens_);
	}

	PARSER_START(TIMOS,meshfile)
	PARSER_START(TIMOS,matfile)
	PARSER_START(TIMOS,sourcefile)


	~TIMOSParserClass()
	{
		if (parser_)
			parser_->free(parser_);
		if (tokens_)
			tokens_->free(tokens_);
		if(lex_)
			lex_->free(lex_);
		if(input_)
			input_->close(input_);
	}
};

void walk(pANTLR3_BASE_TREE bt,unsigned level=0)
{
	assert(bt);
	pANTLR3_COMMON_TOKEN tok = bt->getToken(bt);

	cout << setw((level++)+1) << ' ';

	string toktype = "(unknown)";

	assert(bt->getType);

	unordered_map<int,string>::const_iterator it = toks.find(bt->getType(bt));

	stringstream ss;
	if (it != toks.end())
		ss << it->second << '(' << bt->getType(bt) << "): ";
	else
		ss << "(unknown ##) ";

	if (tok)
		ss << (const char*)tok->getText(tok)->chars;

	cout << ss.str() << endl;

	for(unsigned i=0; i<bt->getChildCount(bt); ++i)
	{
		pANTLR3_BASE_TREE c = (pANTLR3_BASE_TREE)bt->getChild(bt,i);
		if (c)
			walk(c,level);
	}
	level--;
}

