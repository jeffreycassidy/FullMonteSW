/*
 * Test.cpp
 *
 *  Created on: Mar 4, 2015
 *      Author: jcassidy
 */



#include "TIMOSReader.hpp"

#include <string>

using namespace std;

int main(int argc,char **argv)
{
	string fnroot="/home/jcassidy/src/FullMonteSW/data/mouse";

	ANTLRParser<TIMOS::ParserDef> P(fnroot + ".source");

	ANTLR3CPP::base_tree bt = P.parse();

	TIMOS::sourcefile_ast_visitor SV;

	SV.walk(bt);

	ANTLRParser<TIMOS::ParserDef> M(fnroot + ".mesh");
	ANTLR3CPP::base_tree mbt = M.parse<TIMOS::ParserDef::Mesh>();

	TIMOS::meshfile_ast_visitor MV;

	MV.walk(mbt);


	cout << "Read mesh with " << MV.points().size() << " points and " << MV.tetras().size() << " tetras" << endl;

	ANTLRParser<TIMOS::ParserDef> O(fnroot + ".opt");
	TIMOS::optfile_ast_visitor OV;

	OV.walk(O.parse<TIMOS::ParserDef::Mat>());

	for(const auto & m : OV.materials())
		cout << m << endl;

	cout << "Total " << OV.materials().size() << " materials" << endl;

	cout << "Coding by region? " << OV.per_region() << endl;
	cout << "Matched boundary? " << OV.matched() << endl;
	cout << "  External n=" << OV.n_ext() << endl;

	return 0;
}
