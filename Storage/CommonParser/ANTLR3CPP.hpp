#include <antlr3.h>
#include <antlr3defs.h>

#include <cassert>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <tuple>

namespace ANTLR3CPP {

class common_token {

public:
	explicit common_token(pANTLR3_COMMON_TOKEN p) : _p(p){}

	const char*	getText() const { return _p ? (const char*)_p->getText(_p)->chars : "(unknown)"; }
	unsigned 	getType() const { assert(_p); return _p->getType(_p); }

	unsigned	getCharPositionInLine() const { assert(_p); return _p->getCharPositionInLine(_p); }
	unsigned	getLine()				const { assert(_p); return _p->getLine(_p); }

	operator bool() const { return _p; }

private:
	const pANTLR3_COMMON_TOKEN _p;			// common_token is a non-owning copy of a pANTLR3_COMMON_TOKEN pointer
};

class base_tree {

public:

	base_tree 	getChild(unsigned i) 	const
		{ assert(_p); assert(i < _p->getChildCount(_p)); return base_tree((pANTLR3_BASE_TREE)_p->getChild(_p,i)); }

	unsigned 	getChildCount() 		const { assert(_p); return _p->getChildCount(_p); }

	common_token getToken() 			const { assert(_p); return common_token(_p->getToken(_p)); }

	int getTokenType()					const {
		assert(_p);
		if (!getToken())
		{
			assert (_p->getType);
			return _p->getType(_p);
		}
		else
			return getToken().getType();
	}
	const char* getTokenText()			const { return getToken().getText(); }

	explicit base_tree(pANTLR3_BASE_TREE p) : _p(p){}
	base_tree(const base_tree& bt) = default;

	pANTLR3_BASE_TREE get_ptr() const { return _p; }

private:
	const pANTLR3_BASE_TREE _p = nullptr;		// base_tree is a non-owning copy of a pANTLR3_BASE_TREE pointer
};

//class tree_node_flyweight {
//public:
//	virtual void do_visit(base_tree bt,unsigned indent=1) const
//		{ std::cout << std::setw(indent) << ' ' << bt.getToken().getText() << std::endl; };
//
//	virtual void walk(base_tree bt,unsigned indent=1) const {
//		do_visit(bt,indent);
//		for(unsigned i=0; i<bt.getChildCount(); ++i)
//		{
//			base_tree ch = bt.getChild(i);
//			const tree_node_flyweight *fw = get_flyweight(ch.getToken().getType());
//			if (fw)
//				fw->walk(ch,indent+1);
//		}
//	}
//
//	static void register_flyweight(int tokType,const tree_node_flyweight* fw)
//	{
//		if (tokType < 0)
//		{
//			std::cerr << "Trying to register a negative token type!" << std::endl;
//			return;
//		}
//		else if (tokType > _flyweights.size())
//			_flyweights.resize(tokType+1);
//
//		_flyweights[tokType] = fw;
//	}
//
//	const tree_node_flyweight* get_flyweight(int tokType) const {
//		if (tokType < 0 || tokType >= _flyweights.size() || _flyweights[tokType] == nullptr)
//			return _default_flyweight;
//		else
//			return _flyweights[tokType];
//	}
//
//private:
//	static std::vector<const tree_node_flyweight*> 	_flyweights;
//	static const tree_node_flyweight* 				_default_flyweight;
//};
//
//const tree_node_flyweight* tree_node_flyweight::_default_flyweight = new tree_node_flyweight;
//
//std::vector<const tree_node_flyweight*> tree_node_flyweight::_flyweights;


//// convert a string to a given type
template<typename T>T convert_string(std::string s)
{
	T t;
	std::stringstream ss(s);
	ss >> t;
	assert(!ss.fail());
	char c;
	ss >> c;
	assert(ss.eof());
	return t;
}

template<>std::string convert_string(std::string s){ return s; }


class ast_visitor {
protected:
	virtual bool do_validate(){ return true; }
	virtual void do_preorder(ANTLR3CPP::base_tree bt){ }
	virtual void do_postorder(ANTLR3CPP::base_tree bt){}
	virtual void do_expand(ANTLR3CPP::base_tree bt){ visit_children(bt); }

	void visit_children(ANTLR3CPP::base_tree bt)
	{
		for(unsigned i=0; i<bt.getChildCount(); ++i)
			walk(bt.getChild(i));
	}

public:
	void walk(ANTLR3CPP::base_tree bt)
	{
		do_preorder(bt);
		do_expand(bt);
		do_postorder(bt);
	}
};

class print_ast_visitor : public ast_visitor
{
	unsigned indent_=1;
	std::ostream& os_;

public:
	print_ast_visitor(std::ostream& os) : os_(os){}

protected:
	virtual void do_preorder(ANTLR3CPP::base_tree bt)
	{
		os_ << std::setw(indent_++) << ' ' << bt.getTokenText() << std::endl;
	}

	virtual void do_postorder(ANTLR3CPP::base_tree bt)
	{
		os_ << std::setw(--indent_) << ' ' << '\\' << bt.getTokenText() << std::endl;
	}
};




template<unsigned I=0,typename U>
	typename std::enable_if< (I < std::tuple_size<U>::value), void>::type
	convert_tuple_element(const base_tree bt,U& t)
{
	get<I>(t) = convert_string< typename std::tuple_element<I,U>::type>(bt.getChild(I).getToken().getText());
	convert_tuple_element<I+1>(bt,t);
}

template<unsigned I,typename U>
	typename std::enable_if< (I == std::tuple_size<U>::value), void>::type
	convert_tuple_element(const base_tree bt,U&)
		{ }

template<class T>T convert_tuple(const base_tree bt)
{
	T t;
	convert_tuple_element<0,T>(bt,t);
	return t;
}

//
//template<class T>std::vector<T> convert_tuple_vector(const base_tree bt,int tokType)
//{
//	std::vector<T> v;
//
//	for(unsigned i=0; i<bt.getChildCount(); ++i)
//	{
//		base_tree c = bt.getChild(i);
//		assert(c.getToken().getType() == tokType);
//
//		v.push_back(convert_tuple<T>(c));
//	}
//	return v;
//}

//class pinnacle_ast_visitor {
//private:
//	unsigned ind=1;
//
//	void visit_file(const base_tree bt)
//	{
//		++ind;
//		std::map<string,string> pm;
//		cout << setw(ind*2) << ' ' << "FILE" << endl;
//		for(unsigned i=0; i<bt.getChildCount(); ++i)
//		{
//			base_tree c = bt.getChild(i);
//			switch(c.getToken().getType())
//			{
//			case TOK_ROI:
//				visit_roi(c);
//				break;
//			case PROP:
//				visit_prop(c,pm);
//				break;
//			default:
//				cout << "AST ERROR!" << endl;
//			}
//		}
//		cout << setw(ind*2) << ' ' << "/FILE" << endl;
//		--ind;
//	}
//
//	void visit_roi(const base_tree bt)
//	{
//		++ind;
//		std::map<string,string> pm;
//
//		//cout << setw(ind*2) << ' ' << "ROI" << endl;
//		for(unsigned i=0; i<bt.getChildCount(); ++i)
//		{
//			base_tree c = bt.getChild(i);
//			switch(c.getToken().getType())
//			{
//			case PROP:
//				visit_prop(c,pm);
//				break;
//			case CURVE:
//				visit_curve(c);
//				cout << "Curve begins at " << c.getToken().getLine() << ':' << c.getToken().getCharPositionInLine() << endl;
//				break;
//			case SURFACEMESH:
//			case MEANMESH:
//				break;
//			default:
//				cout << "AST ERROR: Expecting PROP or CURVE, received token " << c.getToken().getText() << endl;
//			}
//		}
//		--ind;
//	}
//
//	void visit_curve(const base_tree bt)
//	{
//		++ind;
//		std::vector<std::array<float,3>> P;
//		std::map<string,string> props;
//
//		for(unsigned i=0; i<bt.getChildCount(); ++i)
//		{
//			base_tree c = bt.getChild(i);
//
//			switch(c.getToken().getType()){
//			case PROP:
//				visit_prop(c,props);
//				break;
//
//			case POINTS:
//				P = convert_tuple_vector<std::array<float,3>>(c,POINT);
//				break;
//
//			default:
//				cout << setw(2*ind) << ' ' << "AST ERROR: Expecting PROP or POINTS, received token " << c.getToken().getText() << endl;
//			}
//		}
//
//		cout << "Curve with " << P.size() << " points" << endl;
//		--ind;
//	}
//
//	void visit_prop(const base_tree bt,std::map<string,string>& pm)
//	{
//		++ind;
//		pair<string,string> p = convert_tuple<std::pair<string,string>>(bt);
//		if (!pm.insert(p).second)
//			cout << "ERROR: duplicate property '" << p.first << '\'' << endl;
//		--ind;
//	}
//
//public:
//	void walk(base_tree bt)
//	{
//		visit_file(bt);
//	}
//};
//
//}
//
//int main(int argc,char **argv)
//{
//	string fn="/home/jcassidy/src/FullMonteSW/Pinnacle/plan_HN.roi";
//
//	if (argc > 1)
//		fn = argv[1];
//
//	pANTLR3_INPUT_STREAM input;
//	ppinnacleLexer lex;
//	pANTLR3_COMMON_TOKEN_STREAM tokens;
//	ppinnacleParser parser;
//
//	input = antlr3FileStreamNew((pANTLR3_UINT8)fn.c_str(),ANTLR3_ENC_8BIT);
//	lex = pinnacleLexerNew(input);
//	tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,
//			TOKENSOURCE(lex));
//	parser = pinnacleParserNew(tokens);
//
//	cout << "Starting parse of " << fn << endl;
//
//	pinnacleParser_start_return r = parser->start(parser);
//
//	ANTLR3CPP::pinnacle_ast_visitor p;
//
//	p.walk(ANTLR3CPP::base_tree(r.tree));
//
//	parser->free(parser);
//	tokens->free(tokens);
//	lex->free(lex);
//	input->close(input);
//
//	return 0;
//}

}
