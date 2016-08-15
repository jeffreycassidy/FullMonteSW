/*
 * Histogram.hpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_EMPIRICALCDF_HPP_
#define OUTPUTTYPES_EMPIRICALCDF_HPP_

#include <vector>
#include <functional>

#include <boost/range/algorithm.hpp>


/**
 * Value	Sortable elements
 * Weight	(optional) weight applied to a given element
 *
 *
 * Percentiles are always given as float in [0,1]
 *
 *
 * Probabilities are evaluated in the conventional way, F(x) = Pr(X <= x) so the first bin has weight W_0 / sum_i(w_i) and last
 * bin has F(x)=1.0
 */

template<typename Value=float,typename Weight=float,class Comp=std::less<Value>>class EmpiricalCDF
{
public:
	/// Create empirical CDF from unsorted pairs (v,w) value and weight
	EmpiricalCDF(const std::vector<std::pair<Value,Weight>>& v,const Comp& comp=Comp());

	/// Return the percentile (0-100%) for a given value
	float percentileOfValue(Value v) const;

	/// Return the value
	Value valueAtPercentile(float v) const;

	/// Number of elements in the CDF
	std::size_t		dim() const;

	/// Total weight in the histogram
	Weight			totalWeight() const;

	void print();

	/// Return (x,F(x)) for the i'th element
	std::pair<Weight,Value> operator[](unsigned i) const { return std::make_pair(m_elements[i].value,m_elements[i].cumulativeWeight); }


private:

	/// Sort the vector according to the provided criterion
	void sort();

	/// Take the sorted list and produce the running sums
	void accumulate();

	/// The type of each element in the CDF, including cumulative value
	struct Element {
		Value	value;
		Weight 	weight;
		Weight 	cumulativeWeight;
	};

	std::vector<Element>		m_elements;
	Comp						m_compare=Comp();
};

template<typename Value,typename Weight,class Comp>EmpiricalCDF<Value,Weight,Comp>::
	EmpiricalCDF(const std::vector<std::pair<Value,Weight>>& v,const Comp& comp) :
		m_compare(comp)
{
	m_elements.resize(v.size());
	for(unsigned i=0;i<v.size();++i)
	{
		m_elements[i].value=v[i].first;
		m_elements[i].weight=v[i].second;
		m_elements[i].cumulativeWeight=0;
	}
	sort();
	accumulate();
}

template<typename Value,typename Weight,class Comp>float EmpiricalCDF<Value,Weight,Comp>::percentileOfValue(Value v) const
{
	return 0;
}

template<typename Value,typename Weight,class Comp>Value EmpiricalCDF<Value,Weight,Comp>::valueAtPercentile(float v) const
{
	return 0;
}

template<typename Value,typename Weight,class Comp>Weight EmpiricalCDF<Value,Weight,Comp>::totalWeight() const
{
	return m_elements.back().cumulativeWeight;
}

template<typename Value,typename Weight,class Comp>std::size_t EmpiricalCDF<Value,Weight,Comp>::dim() const
{
	return m_elements.size();
}

template<typename Value,typename Weight,class Comp>void EmpiricalCDF<Value,Weight,Comp>::sort()
{
	Comp comp = m_compare;
	boost::sort(m_elements, [comp](Element lhs,Element rhs){ return comp(lhs.value,rhs.value); });
}

template<typename Value,typename Weight,class Comp>void EmpiricalCDF<Value,Weight,Comp>::accumulate()
{
	Weight cw=0;

	for(auto& e : m_elements)
		e.cumulativeWeight = (cw += e.weight);
}

#include <iostream>
#include <iomanip>
using namespace std;

template<typename Value,typename Weight,class Comp>void EmpiricalCDF<Value,Weight,Comp>::print()
{
	cout << setprecision(2) << fixed;

	Weight cw=m_elements.back().cumulativeWeight;

	cout << "x         |  F(x) %" << endl;

	for(const auto& e : m_elements)
		cout << setprecision(4) << scientific << setw(10) << e.value << "  " << fixed << setprecision(3) << 100.0f*e.cumulativeWeight/cw << "% " << endl;

}



#endif /* OUTPUTTYPES_EMPIRICALCDF_HPP_ */
