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

template<typename T,size_t N>ostream& operator<<(ostream& os,const array<T,N>& a)
{
	os << a[0];
	for(size_t i=1;i<N;++i)
		os << ' ' << a[i];
	return os;
}

class TreeWalker {
	list<map<string,string>> propmap_list;
	list<vector<array<double,3>>> pointvec_list;

	map<string,string>* propmap_current;
	vector<array<double,3>>* pointvec_current;

	bool handleProp(pANTLR3_BASE_TREE tree,unsigned indent);
	bool handlePoint(pANTLR3_BASE_TREE tree,unsigned indent);


public:
	TreeWalker() : propmap_list(1),pointvec_list(1),propmap_current(&propmap_list.front()),pointvec_current(&pointvec_list.front()){}
    int run(pANTLR3_BASE_TREE,unsigned indent=0);

	void printAll() const;
};

pANTLR3_BASE_TREE getChild(pANTLR3_BASE_TREE, unsigned);
const char* getText(pANTLR3_BASE_TREE tree);

int main(int argc, char* argv[])
{
  pANTLR3_INPUT_STREAM input;
  ppinnacleLexer lex;
  pANTLR3_COMMON_TOKEN_STREAM tokens;
  ppinnacleParser parser;

  assert(argc > 1);
  input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)argv[1]);
  lex = pinnacleLexerNew(input);
  tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
                                            TOKENSOURCE(lex));
  parser = pinnacleParserNew(tokens);

  cout << "Starting parse of " << argv[1] << endl;

  pinnacleParser_start_return r = parser->start(parser);

  pANTLR3_BASE_TREE tree = r.tree;

  TreeWalker wk;

  cout << endl;

  wk.run(tree);

  wk.printAll();

  parser->free(parser);
  tokens->free(tokens);
  lex->free(lex);
  input->close(input);

  return 0;
}

void TreeWalker::printAll() const
{
	unsigned i=0;

	cout << "There are " << pointvec_list.size() << " point vectors with lengths: " << endl;
	for(const vector<array<double,3>>& v : pointvec_list)
		cout << v.size() << endl;

	cout << endl << "There are " << propmap_list.size() << " property maps with sizes: " << endl;
	for(const map<string,string>& m : propmap_list)
		cout << m.size() << endl;


	i=0;
	for(const vector<array<double,3>>& v : pointvec_list)
	{
		cout << setfill('*') << setw(80) << "" << endl << "** Point vector " << i++ << endl;
		for(const array<double,3>& a : v)
			cout << a << endl;
	}

	i=0;
	for(const map<string,string>& m : propmap_list)
	{
		cout << setfill('*') << setw(80) << "" << endl << "** Property map " << i++ << endl;
		for(const pair<string,string>& s : m)
			cout << s.first << " => " << s.second << endl;
	}
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
	bool inserted;
	map<string,string>::const_iterator it;

	string k=getChildText(tree,0),v=getChildText(tree,1);

	// clip off leading colon and trailing newline if it's a literal
	if (getChildType(tree,1) == LITERAL)
	{
		assert(v[0] == ':');
		size_t first_nonspace = v.find_first_not_of(" \t",1);
		v=v.substr(first_nonspace,v.size()-first_nonspace-1);
	}

	// insert into property map
	tie(it,inserted) = propmap_current->insert(make_pair(k,v));

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
	
	cout << setw(indent+1) << ' ' << "Point: " << a << endl;
	pointvec_current->push_back(a);
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
				propmap_list.push_back(map<string,string>());
				propmap_current = &propmap_list.back();

				pointvec_list.push_back(vector<array<double,3>>());
				pointvec_current = &pointvec_list.back();
			break;

			case ROI:
			break;
		}

		int k = tree->getChildCount(tree);
		int r=0;
		for(int i=0;i<k;++i)
			r=run((pANTLR3_BASE_TREE)tree->getChild(tree,i),indent+1);
		return r;
	}
	else
		cout << "NOT A TOKEN!" << endl;
	return 0;
}
