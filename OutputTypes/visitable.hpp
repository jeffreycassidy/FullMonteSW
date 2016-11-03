/*
 * visitable.hpp
 *
 *  Created on: Oct 20, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_VISITABLE_HPP_
#define OUTPUTTYPES_VISITABLE_HPP_

#ifndef SWIG
#define ACCEPT_VISITOR_METHOD(Base,Derived) virtual void acceptVisitor(Base::Visitor* v) override { v->doVisit(static_cast<Derived*>(this)); }
#else
#define ACCEPT_VISITOR_METHOD(Base,Derived) //virtual void acceptVisitor(Base::Visitor* v) override;
#endif

template<class VisitorT>class visitable_base
{
public:
	typedef VisitorT Visitor;
	virtual void acceptVisitor(Visitor* v)=0;
	virtual ~visitable_base(){}

protected:
	visitable_base(){}
};

template<class Base,class Derived,class Visitor=typename Base::Visitor>class visitable : virtual public visitable_base<Visitor>
{
protected:
	visitable(){}
	~visitable(){}

public:
	virtual void acceptVisitor(Visitor* v)
	{
		v->doVisit(static_cast<Derived*>(this));
	}

	friend Visitor;
};




/** Classes deriving from visitor<B> should implement doVisit(D* d) for all derived classes of B.
 *
 *
 * Base must be derived from visitable_base<T>
 */

template<class Base,class Visitor>class visitor
{
public:
	virtual void visit(Base* B){ B->acceptVisitor(static_cast<Visitor*>(this)); }

protected:
	visitor(){}
	virtual ~visitor(){}

private:
	// void doVisit(D* d)
};



#endif /* OUTPUTTYPES_VISITABLE_HPP_ */
