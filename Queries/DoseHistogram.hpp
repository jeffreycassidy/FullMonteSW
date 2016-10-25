/*
 * DoseHistogram.hpp
 *
 *  Created on: Oct 21, 2016
 *      Author: jcassidy
 */

#ifndef QUERIES_DOSEHISTOGRAM_HPP_
#define QUERIES_DOSEHISTOGRAM_HPP_

#include <FullMonteSW/OutputTypes/OutputData.hpp>
#include <FullMonteSW/OutputTypes/clonable.hpp>
#include "EmpiricalCDF.hpp"

class DoseHistogram : public OutputData
{
public:
	DoseHistogram(const std::vector<std::pair<float,float>>& v) :
		m_histogram(v)
	{}

#ifndef SWIG
	CLONE_METHOD(OutputData,DoseHistogram)
	virtual const char* typeString() const override { return "DoseHistogram"; }
#endif

	void print() const;

	std::pair<float,float> operator[](std::size_t i) const
	{
		return m_histogram[i];
	}


	std::size_t dim() const { return m_histogram.dim(); }


private:
	EmpiricalCDF<float,float>		m_histogram;
};

inline void DoseHistogram::print() const
{
	m_histogram.print();
}


#endif /* QUERIES_DOSEHISTOGRAM_HPP_ */
