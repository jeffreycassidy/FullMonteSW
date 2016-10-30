/*
 * Abstract.cpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#include "Abstract.hpp"
#include "Composite.hpp"

namespace Source {

Abstract::Visitor::Visitor()
{
}

Abstract::Visitor::~Visitor()
{

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

};
