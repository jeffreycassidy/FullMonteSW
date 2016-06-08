/*
 * AbsorptionSum.cpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#include "AbsorptionSum.hpp"

#include <boost/range/counting_range.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <iomanip>

const AlwaysTrue<unsigned> AbsorptionSum::s_alwaysTrue;

AbsorptionSum::AbsorptionSum()
{

}

float AbsorptionSum::compute() const
{
	float sum=0.0f;
	unsigned N=0;

	for(unsigned i : boost::counting_range<unsigned>(0U, (*m_E)->dim())
			| boost::adaptors::filtered([this](unsigned i){ return (*m_filter)(i); }))
	{
		sum += (*m_E)[i];
		++N;
	}
	cout << "AbsorptionSum: " << N << " elements summed totaling " << std::fixed << std::setprecision(6) << sum << endl;
	return sum;
}
