/*
 * NIRFAST.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jcassidy
 */

#include "NIRFASTLexer.h"
#include "NIRFASTParser.h"

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
#include "NIRFAST_tokens.h"
};


class NIRFASTParserClass {
	pANTLR3_INPUT_STREAM input_=nullptr;
	pNIRFASTLexer lex_=nullptr;
	pANTLR3_COMMON_TOKEN_STREAM tokens_=nullptr;
	pNIRFASTParser parser_=nullptr;

public:

	NIRFASTParserClass(string fn)
	{
		input_ = antlr3FileStreamNew((pANTLR3_UINT8)fn.c_str(),ANTLR3_ENC_8BIT);
		lex_ = NIRFASTLexerNew(input_);
		tokens_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
				TOKENSOURCE(lex_));
		parser_ = NIRFASTParserNew(tokens_);
	}

	NIRFASTParser_nodefile_return parse_node()
	{
		return parser_->nodefile(parser_);
	}

	NIRFASTParser_elemfile_return parse_elem()
	{
		return parser_->elemfile(parser_);
	}

	NIRFASTParser_paramfile_return parse_param()
	{
		return parser_->paramfile(parser_);
	}


	NIRFASTParser_measfile_return parse_meas()
	{
		return parser_->measfile(parser_);
	}

	NIRFASTParser_sourcefile_return parse_source()
		{
			return parser_->sourcefile(parser_);
		}

	NIRFASTParser_regionfile_return parse_region()
		{
			return parser_->regionfile(parser_);
		}

	NIRFASTParser_linkfile_return parse_link()
			{
				return parser_->linkfile(parser_);
			}

	~NIRFASTParserClass()
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

int main(int argc,char **argv)
{
	string fnroot="/home/jcassidy/src/nirfast-read-only/meshes/standard/cylinder_stnd";

	if (argc > 1)
		fnroot = argv[1];

	NIRFASTParserClass node(fnroot+".node");
	NIRFASTParserClass elem(fnroot+".elem");
	NIRFASTParserClass param(fnroot+".param");
	NIRFASTParserClass meas(fnroot+".meas");

	NIRFASTParserClass region(fnroot+".region");

	NIRFASTParserClass source(fnroot+".source");
	NIRFASTParserClass link(fnroot+".link");

	walk(node.parse_node().tree);
	walk(elem.parse_elem().tree);
	walk(param.parse_param().tree);
	walk(meas.parse_meas().tree);
	walk(source.parse_source().tree);
	walk(region.parse_region().tree);
	walk(link.parse_link().tree);

	return 0;
}


