/*
 * AbsorptionSum.cpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#include "AbsorptionSum.hpp"

#include <boost/range/counting_range.hpp>
#include <boost/range/adaptor/filtered.hpp>

const AlwaysTrue<unsigned> AbsorptionSum::s_alwaysTrue;

AbsorptionSum::AbsorptionSum()
{

}

float AbsorptionSum::compute() const
{
	float sum=0.0f;
	for(unsigned i : boost::counting_range<unsigned>(0U, (*m_E)->dim())
			| boost::adaptors::filtered([this](unsigned i){ return (*m_filter)(i); }))
		sum += (*m_E)[i];
	return sum;
}
