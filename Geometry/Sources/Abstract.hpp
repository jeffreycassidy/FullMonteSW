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

#ifndef SWIG
	// Visitor methods
	class Visitor
	{
	public:
		Visitor();
		virtual ~Visitor();

		virtual void visit(Abstract* B){ B->acceptVisitor(this); }

		virtual void doVisit(Point* p)			=0;
		virtual void doVisit(Ball* b)			=0;
		virtual void doVisit(Line* l)			=0;
		virtual void doVisit(Volume* v)			=0;

		/// Default visit for Composite type: call pre-visit handler, visit all sub-sources, and then post-visit handler
		/// Can be overridden.
		virtual void doVisit(Composite* c);

		virtual void doVisit(Surface* s)		=0;
		virtual void doVisit(SurfaceTri* st)	=0;
		virtual void doVisit(Abstract* b)		=0;
		virtual void doVisit(PencilBeam* b)		=0;

		virtual void preVisitComposite(Composite* C){}
		virtual void postVisitComposite(Composite* C){}

	private:
		unsigned	m_compositeLevel=0;
		unsigned 	m_maxCompositeLevel=-1U;
	};

	virtual void acceptVisitor(Visitor* v){ v->doVisit(this); }
#endif

	virtual Abstract* clone() const{ return new Abstract(*this); }

	float 		power() 		const 	{ return m_power; 	}
	void 		power(float p)			{ m_power=p; 		}

private:
	float m_power=1.0f;
};

};

#endif /* GEOMETRY_SOURCES_BASE_HPP_ */
