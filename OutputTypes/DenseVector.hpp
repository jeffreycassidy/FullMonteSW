/*
 * DenseVector.hpp
 *
 *  Created on: Nov 13, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_DENSEVECTOR_HPP_
#define OUTPUTTYPES_DENSEVECTOR_HPP_

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>

/* Basic operations required of a fluence container
nonzeros()
dense()
dim()
nnz()
const Value& operator[]
*/

//template<typename T>bool value_nonzero(const T v){ return v != 0; }
template<typename T,typename Index=std::ptrdiff_t>bool value_nonzero(const boost::range::index_value<const T&,Index>& iv)
	{ return iv.value() != 0; }


template<typename T>class DenseVector
{
public:
	DenseVector(){}
	DenseVector(std::vector<T>&& v) : m_v(v)
		{ countNonZero(); }
	template<class Alloc>DenseVector(const std::vector<T,Alloc>& v) : m_v(v)
		{ countNonZero(); }

	typedef decltype(std::declval<std::vector<T>>() | boost::adaptors::indexed(0U)) const_dense_iterator;
	typedef decltype(std::declval<std::vector<T>>() | boost::adaptors::indexed(0U) | boost::adaptors::filtered(value_nonzero<T>)) const_sparse_iterator;

	decltype(std::declval<std::vector<T>>() | boost::adaptors::indexed(0U)) dense() const
		{ return m_v | boost::adaptors::indexed(0U); }

	decltype(std::declval<std::vector<T>>()  | boost::adaptors::indexed(0U) | boost::adaptors::filtered(value_nonzero<T>)) nonzeros() const
		{ return m_v | boost::adaptors::indexed(0U) | boost::adaptors::filtered(value_nonzero<T>); }

	std::size_t nnz() const { return m_nnz; }
	std::size_t dim() const { return m_v.size(); }

private:
	void countNonZero();
	std::size_t m_nnz=0;
	std::vector<T> m_v;
};

template<typename T>void DenseVector<T>::countNonZero()
{
	m_nnz = boost::size(nonzeros());
}



#endif /* OUTPUTTYPES_DENSEVECTOR_HPP_ */
