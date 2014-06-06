#include "PinnacleFile.hpp"
#include "PinnacleROI.hpp"
#include "PinnacleCurve.hpp"

#include "pinnacleLexer.h"
#include "pinnacleParser.h"

#include <cassert>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>

#include <list>
#include <vector>
#include <array>
#include <tuple>

using namespace std;

namespace Pinnacle {

string getChildText(pANTLR3_BASE_TREE p,unsigned i);
int getChildType(pANTLR3_BASE_TREE p,unsigned i);


template<typename T,size_t N>ostream& operator<<(ostream& os,const array<T,N>& a)
{
	os << a[0];
	for(size_t i=1;i<N;++i)
		os << ' ' << a[i];
	return os;
}

class TreeWalker {
	Pinnacle::File* current_file=NULL;
	Pinnacle::Object* current_props=NULL;
	Pinnacle::ROI* current_roi = NULL;
	Pinnacle::Curve* current_curve=NULL;

	bool handleProp(pANTLR3_BASE_TREE tree,unsigned indent);
	bool handlePoint(pANTLR3_BASE_TREE tree,unsigned indent);

public:
	TreeWalker(Pinnacle::File *f) : current_file(f),current_props(f){}
    int run(pANTLR3_BASE_TREE,unsigned indent=0);

    void printAll() const {
    	current_file->printDetails();
    }

};




bool Pinnacle::File::read()
{
	pANTLR3_INPUT_STREAM input;
	ppinnacleLexer lex;
	pANTLR3_COMMON_TOKEN_STREAM tokens;
	ppinnacleParser parser;

	input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)fn.c_str());
	lex = pinnacleLexerNew(input);
	tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
			TOKENSOURCE(lex));
	parser = pinnacleParserNew(tokens);

	cout << "Starting parse of " << fn << endl;

	pinnacleParser_start_return r = parser->start(parser);

	pANTLR3_BASE_TREE tree = r.tree;

	cout << endl;

	TreeWalker wk(this);

	wk.run(tree);
	wk.printAll();

	parser->free(parser);
	tokens->free(tokens);
	lex->free(lex);
	input->close(input);

	buildSliceMap();

	return true;
}






string getChildText(pANTLR3_BASE_TREE p,unsigned i)
{
	assert(p->getChildCount(p) > i);
	pANTLR3_BASE_TREE child = (pANTLR3_BASE_TREE)p->getChild(p,i);
	pANTLR3_COMMON_TOKEN child_tok = (pANTLR3_COMMON_TOKEN)child->getToken(child);
	return string((const char*)child_tok->getText(child_tok)->chars);
}

int getChildType(pANTLR3_BASE_TREE p,unsigned i)
{
	assert(p->getChildCount(p) > i);
	pANTLR3_BASE_TREE child = (pANTLR3_BASE_TREE)p->getChild(p,i);
	pANTLR3_COMMON_TOKEN child_tok = (pANTLR3_COMMON_TOKEN)child->getToken(child);
	return child_tok->type;
}

bool TreeWalker::handleProp(pANTLR3_BASE_TREE tree,unsigned indent)
{
	map<string,string>::const_iterator it;

	string k=getChildText(tree,0),v=getChildText(tree,1);

	// clip off leading colon and trailing newline if it's a literal
	if (getChildType(tree,1) == LITERAL)
	{
		assert(v[0] == ':');
		size_t first_nonspace = v.find_first_not_of(" \t",1);
		v=v.substr(first_nonspace,v.size()-first_nonspace-1);
	}

	current_props->addprop(k,v);

	return true;
}

bool TreeWalker::handlePoint(pANTLR3_BASE_TREE tree,unsigned indent)
{
	array<double,3> a;
	for(unsigned i=0;i<3;++i)
	{
		pANTLR3_BASE_TREE child = (pANTLR3_BASE_TREE)tree->getChild(tree,i);
		pANTLR3_COMMON_TOKEN child_tok = (pANTLR3_COMMON_TOKEN)child->getToken(child);
		if (child_tok->type != FLOAT || child->getChildCount(child) != 0)
			cerr << "Unexpected tree structure at point element" << endl;
		sscanf((const char*)child_tok->getText(child_tok)->chars,"%lf",&a[i]);
	}

	//cout << setw(indent+1) << ' ' << "Point: " << a << endl;

	current_curve->addpoint(a);
	return true;
}

int TreeWalker::run(pANTLR3_BASE_TREE tree,unsigned indent)
{
	unsigned Nchildren;
	pANTLR3_COMMON_TOKEN tok = tree->getToken(tree);

	if (tok){
		Nchildren = tree->getChildCount(tree);

		switch(tok->type){
		case POINT:
			if (Nchildren != 3)
			{
				cerr << "Invalid number of children for POINT: expecting 3, found " << Nchildren << endl;
				return -1;
			}

			if(!handlePoint(tree,indent))
			{
				cerr << "Error handling a POINT" << endl;
				return -1;
			}


			break;

		case PROP:
			if (Nchildren != 2)
			{
				cerr << "Invalid number of children for PROP: expecting 2, found " << Nchildren << endl;
				return -1;
			}

			if(!handleProp(tree,indent))
			{
				cerr << "Error handling a PROP" << endl;
				return -1;
			}
			break;

		case CURVE:
			current_props=current_curve=current_roi->newcurve();
			break;

		case TOK_ROI:
			current_props=current_roi=current_file->newroi();
			break;
		}

		int k = tree->getChildCount(tree);
		int r=0;
		for(int i=0;i<k;++i)
			r=run((pANTLR3_BASE_TREE)tree->getChild(tree,i),indent+1);
		return r;
	}
	else
		cerr << "NOT A TOKEN!" << endl;
	return 0;
}

void File::printDetails() const
{
	//unsigned i=0;

	cout << "There are " << rois.size() << " ROIs defined; details: " << endl;

	for(const ROI& r : rois)
	{
		//cout << "  [" << setw(4) << i++ << "] ";
		//r.printDetails();
	}

	//cout << "There are " << M.size() << " slices; details: " << endl;
}



/** Maps curves to slices based on z coordinate.
 *
 */

void File::buildSliceMap()
{
	M.clear();

	for(const Curve& c : getCurves())
		M.insert(make_pair(c.getPoints().front()[2],0));

	// give slices one-based ID
	unsigned i=1;
	for(pair<const double,unsigned>& sl : M)
		sl.second = i++;

	// assign Curves to slices
	for(ROI& r : rois)
		for(Curve& c : r.curves)
			c.sliceID = M.at(c.getPoints().front()[2]);

}

void File::printSliceMap() const
{
	cout << "Slices: " << endl;
	for(auto m : M)
		cout << "  [" << setw(4) << m.second << "] " << m.first << endl;
}

}

