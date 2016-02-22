#ifndef ANTLRPARSER_INCLUDED
#define ANTLRPARSER_INCLUDED

#include <antlr3.h>

#include "ANTLR3CPP.hpp"

#include <string>
#include <cassert>
#include <sstream>

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

		if (!input_)
		{
			std::stringstream ss;
			ss << "ANTLRParser: failed to open file '" << fn.c_str();
			throw std::ios_base::failure(ss.str());
		}

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

#endif
