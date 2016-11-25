/*
 * Abstract.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#include "Abstract.hpp"
#include "Composite.hpp"
#include "Line.hpp"
#include "Ball.hpp"
#include "Volume.hpp"
#include "Surface.hpp"
#include "SurfaceTri.hpp"
#include "PencilBeam.hpp"

namespace Source {

Abstract::Visitor::Visitor()
{
}

Abstract::Visitor::~Visitor()
{

}

void Abstract::acceptVisitor(Visitor* V)
{
	V->doVisit(this);
}

void Abstract::Visitor::doVisit(Source::Composite* C)
{
	preVisitComposite(C);
	if (m_compositeLevel < m_maxCompositeLevel)
	{
		++m_compositeLevel;
		for(Abstract* a : C->elements())
			a->acceptVisitor(this);
		--m_compositeLevel;
	}
	postVisitComposite(C);
}

void Abstract::Visitor::doVisit(Source::Line* L)
{
	undefinedVisitMethod(L);
}

void Abstract::Visitor::doVisit(Source::Ball* B)
{
	undefinedVisitMethod(B);
}

void Abstract::Visitor::doVisit(Source::Point* P)
{
	undefinedVisitMethod(P);
}

void Abstract::Visitor::doVisit(Source::PencilBeam* PB)
{
	undefinedVisitMethod(PB);
}

void Abstract::Visitor::doVisit(Source::Surface* S)
{
	undefinedVisitMethod(S);
}

void Abstract::Visitor::doVisit(Source::SurfaceTri* ST)
{
	undefinedVisitMethod(ST);
}

void Abstract::Visitor::doVisit(Source::Volume* V)
{
	undefinedVisitMethod(V);
}

};
