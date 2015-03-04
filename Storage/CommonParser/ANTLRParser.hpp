/*
 * NIRFAST.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jcassidy
 */

#include <antlr3.h>
#include <antlr3defs.h>

#include "ANTLR3CPP.hpp"

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <sstream>
#include <vector>

#include <unordered_map>

template<class ANTLR3LP>class ANTLRParser {
	pANTLR3_INPUT_STREAM 				input_=nullptr;
	pANTLR3_COMMON_TOKEN_STREAM 		tokens_=nullptr;

	typename ANTLR3LP::Lexer::ptr_t 	lex_=nullptr;
	typename ANTLR3LP::Parser::ptr_t	parser_=nullptr;

protected:

public:
	ANTLRParser(std::string fn)
	{
		input_ 	= antlr3FileStreamNew((pANTLR3_UINT8)fn.c_str(),ANTLR3_ENC_8BIT);
		assert(input_);

		lex_ 	= ANTLR3LP::Lexer::CreateLexer(input_);
		assert(lex_);

		tokens_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
				lex_->pLexer->rec->state->tokSource);
		assert(tokens_);

		parser_ = ANTLR3LP::Parser::CreateParser(tokens_);
		assert(parser_);
	}

	template<class T=typename ANTLR3LP::Parser>ANTLR3CPP::base_tree parse()
	{
		auto ret = T::startrule(parser_)(parser_);
		assert(ret.tree);
		return ANTLR3CPP::base_tree(ret.tree);
	}

	virtual ~ANTLRParser()
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
