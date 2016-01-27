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

	class ANTLR3LP;
	class sourcefile_ast_visitor;
	class meshfile_ast_visitor;
	class optfile_ast_visitor;
	class legendfile_ast_visitor;

	virtual Optical 					parse_optical(std::string fn)	const override;
	virtual std::vector<SourceDef>		parse_sources(std::string fn)	const override;
	virtual Mesh						parse_mesh(std::string fn)		const override;
	virtual std::vector<LegendEntry> 	parse_legend(std::string fn)	const override;

	//static const std::unordered_map<int,std::string> toks;
};





#endif /* STORAGE_TIMOS_TIMOSANTLRPARSER_HPP_ */
