/*
 * Composite.hpp
 *
 *  Created on: Jan 26, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_SOURCES_COMPOSITE_HPP_
#define GEOMETRY_SOURCES_COMPOSITE_HPP_

#include "Base.hpp"
#include <vector>

namespace Source {

class Composite : public Source::detail::cloner<Base,Composite>
{
public:
	// Create from a pair of iterators that dereference to a Source*
	SourceMultiDescription(){}
	template<class ConstIterator> SourceMultiDescription(ConstIterator begin,ConstIterator end) :
			sources(begin,end),
			w_total(0.0)
			{ for(; begin != end; ++begin) w_total += (*begin)->getPower(); }

private:
	std::vector<Base*>		m_elements;
};

};

#endif /* GEOMETRY_SOURCES_COMPOSITE_HPP_ */
