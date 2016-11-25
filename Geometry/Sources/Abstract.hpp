/*
 * Base.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_BASE_HPP_
#define GEOMETRY_SOURCES_BASE_HPP_

#include <FullMonteSW/OutputTypes/clonable.hpp>
#include <FullMonteSW/OutputTypes/visitable.hpp>

#ifndef SWIG
#define DERIVED_SOURCE_MACRO(Base,Derived) CLONE_METHOD(Base,Derived) \
	ACCEPT_VISITOR_METHOD(Base,Derived)
#else
#define DERIVED_SOURCE_MACRO(Base,Derived)
#endif

namespace Source {

/** Abstract source base class with the minimum required items (virtual destructor, visitor facilities, power) */

class PencilBeam;
class Point;
class Ball;
class Line;
class Volume;
class Surface;
class SurfaceTri;
class Abstract;
class Composite;

class Abstract
{
public:
	Abstract(float p=1.0f) : m_power(p){}
	virtual ~Abstract(){}

	class Visitor;

	virtual void acceptVisitor(Visitor* V);

	virtual Abstract* clone() const{ return new Abstract(*this); }

	float 		power() 		const 	{ return m_power; 	}
	void 		power(float p)			{ m_power=p; 		}

private:
	float m_power=1.0f;
};


#ifndef SWIG
// Visitor methods
class Abstract::Visitor
{
public:
	Visitor();
	virtual ~Visitor();

	void visit(Abstract* B){ B->acceptVisitor(this); }

	void doVisit(Abstract* A){ undefinedVisitMethod(A); }

	virtual void doVisit(Point* p);
	virtual void doVisit(Ball* b);
	virtual void doVisit(Line* l);
	virtual void doVisit(Volume* v);

	/// Visit for Composite type: call pre-visit handler, visit all sub-sources, and then post-visit handler
	void doVisit(Composite* c);

	virtual void doVisit(Surface* s);
	virtual void doVisit(SurfaceTri* st);
	virtual void doVisit(PencilBeam* b);

	virtual void preVisitComposite(Composite* C){}
	virtual void postVisitComposite(Composite* C){}

	virtual void undefinedVisitMethod(Abstract*){}

private:
	unsigned	m_compositeLevel=0;
	unsigned 	m_maxCompositeLevel=-1U;
};

#endif

};

#endif /* GEOMETRY_SOURCES_BASE_HPP_ */
