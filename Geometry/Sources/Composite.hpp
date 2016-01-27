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
#include <boost/range.hpp>

namespace Source {

class Composite : public Source::detail::cloner<Base,Composite>
{
public:
	explicit Composite(float w=1.0) : cloner(w){}
	Composite(float w,std::vector<Base*>&& srcs) : cloner(w),m_elements(srcs){}

	boost::iterator_range<std::vector<Source::Base*>::iterator> elements()
		{
		return boost::iterator_range<std::vector<Source::Base*>::iterator>( m_elements.begin(), m_elements.end() );
		}

private:
	std::vector<Base*>		m_elements;
};

};

#endif /* GEOMETRY_SOURCES_COMPOSITE_HPP_ */
