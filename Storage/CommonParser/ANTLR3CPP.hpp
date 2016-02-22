#ifndef ANTLR3CPP_INCLUDED
#define ANTLR3CPP_INCLUDED

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

template<>inline std::string convert_string(std::string s){ return s; }


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
	virtual ~ast_visitor(){}
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
	std::get<I>(t) = convert_string< typename std::tuple_element<I,U>::type>(bt.getChild(I).getToken().getText());
	convert_tuple_element<I+1>(bt,t);
}

template<unsigned I,typename U>
	typename std::enable_if< (I == std::tuple_size<U>::value), void>::type
	convert_tuple_element(const base_tree bt,U&)
		{ }

template<class T>T convert_tuple(const base_tree bt)
{
	T t;
	assert(bt.getChildCount() == std::tuple_size<T>::value);
	convert_tuple_element<0,T>(bt,t);
	return t;
}




}
#endif
