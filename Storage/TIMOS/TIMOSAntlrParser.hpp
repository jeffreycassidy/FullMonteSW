/*
 * TIMOSParser.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef STORAGE_TIMOS_TIMOSANTLRPARSER_HPP_
#define STORAGE_TIMOS_TIMOSANTLRPARSER_HPP_

#include "../CommonParser/ANTLRParser.hpp"
#include "TIMOSReader.hpp"

class TIMOSAntlrParser : public TIMOSReader
{

public:
	TIMOSAntlrParser(){}
	TIMOSAntlrParser(const std::string pfx) : TIMOSReader(pfx){}

	virtual ~TIMOSAntlrParser(){}

private:

	virtual Optical 					parse_optical(std::string fn)	const override;
	virtual std::vector<SourceDef>		parse_sources(std::string fn)	const override;
	virtual Mesh						parse_mesh(std::string fn)		const override;
	virtual std::vector<LegendEntry> 	parse_legend(std::string fn)	const override;

	//static const std::unordered_map<int,std::string> toks;

	class sourcefile_ast_visitor;
	class meshfile_ast_visitor;
	class optfile_ast_visitor;
	class legendfile_ast_visitor;

#ifndef SWIG
	MAKE_LEXER_DEF(TIMOS)
	MAKE_PARSER_DEF(TIMOS,sourcefile)

	ADD_START_RULE(TIMOS,Mat,matfile)
	ADD_START_RULE(TIMOS,Mesh,meshfile)
	ADD_START_RULE(TIMOS,Source,sourcefile)
	ADD_START_RULE(TIMOS,Legend,legendfile)
#endif

	friend class ANTLRParser<TIMOSAntlrParser>;
};





#endif /* STORAGE_TIMOS_TIMOSANTLRPARSER_HPP_ */
