/*
 * SparseVector.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_SPARSEVECTOR_HPP_
#define OUTPUTTYPES_SPARSEVECTOR_HPP_

#include <boost/container/flat_map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <vector>
#include <iostream>

#include <fstream>

#include "SparseVector.hpp"

/**
 * @tparam	Index		The index type.
 * @tparam	Value		The value type. Must be equality-comparable, copyable/assignable, and default-constructible.
 */

template<typename Index,typename Value> class SparseVector
{
public:
	SparseVector(){}

	/// Construct from an Iterator range, with optional hint giving number of non-zero values
	template<class Iterator>SparseVector(Iterator begin,Iterator end,std::size_t N,std::size_t nnz=0);

	/// Return a dense representation, with optional return conversion to ReturnValue (defaults to Value)
	template<typename ReturnValue=Value>std::vector<ReturnValue> denseVector() const
	{
		std::vector<ReturnValue> v(m_N,0);
		for(const auto p : m_contents)
		{
			assert(p.first < v.size());
			v[p.first] = p.second;
		}
		return v;
	}


	/** Adapted iterator that returns boost::index_value<ReturnValue,Index> with index and value for nonzero values.
	 */

	template<typename ReturnValue=Value>class const_sparse_iterator : public boost::iterator_adaptor<
			const_sparse_iterator<ReturnValue>,
			typename boost::container::flat_map<Index,Value>::const_iterator,
			boost::range::index_value<const ReturnValue,Index>,
			boost::forward_traversal_tag,
			boost::range::index_value<const ReturnValue,Index>,
			std::ptrdiff_t>
	{
	public:
		const_sparse_iterator(const const_sparse_iterator&) = default;
		const_sparse_iterator(const typename boost::container::flat_map<Index,Value>::const_iterator& it) : const_sparse_iterator::iterator_adaptor_(it){}

		boost::range::index_value<const ReturnValue,Index> dereference() const
		{
			Index i = this->base_reference()->first;
			ReturnValue v = this->base_reference()->second;

			return boost::range::index_value<const ReturnValue,Index>(i,v);
		}

	private:
		friend class boost::iterator_core_access;
	};



	/** Adapted iterator that returns boost::index_value<ReturnValue,Index> for all elements.
	 */

	template<typename ReturnValue=Value>class const_dense_iterator : public boost::iterator_adaptor<
			const_dense_iterator<ReturnValue>,
			typename boost::container::flat_map<Index,Value>::const_iterator,
			boost::range::index_value<const ReturnValue,Index>,
			boost::forward_traversal_tag,
			boost::range::index_value<const ReturnValue,Index>,
			std::ptrdiff_t>
	{
	public:
		typedef typename boost::iterator_adaptor<
				const_dense_iterator<ReturnValue>,
				typename boost::container::flat_map<Index,Value>::const_iterator,
				boost::range::index_value<const ReturnValue,Index>,
				boost::forward_traversal_tag,
				boost::range::index_value<const ReturnValue,Index>,
				std::ptrdiff_t> Super;

		typedef typename boost::container::flat_map<Index,Value>::const_iterator Base;

		using Super::base_reference;

		const_dense_iterator(const const_dense_iterator&) = default;
		const_dense_iterator(const Base& it,const Base& itEnd,Index idx=0) :
			const_dense_iterator::iterator_adaptor_(it),
			m_itEnd(itEnd),
			m_Idx(idx){}

		bool equal(const const_dense_iterator& it) const
		{
			return it.m_Idx == m_Idx;
		}

		void increment()
		{
			++m_Idx;

			if (base_reference() == m_itEnd)
				return;
			else if (base_reference()->first < m_Idx)
				base_reference()++;

			if (base_reference() != m_itEnd && base_reference()->first == m_Idx)
				m_val = base_reference()->second;
			else
				m_val = Value();
		}

		boost::range::index_value<const ReturnValue,Index> dereference() const
		{
			return boost::range::index_value<const ReturnValue,Index>(m_Idx,m_val);
		}

	private:
		const Base	m_itEnd;
		Index 		m_Idx=0;
		ReturnValue	m_val=0;
		friend class boost::iterator_core_access;
	};


	/** Provides a range of boost::index_value<Value,Index> holding the nonzeros.
	 */

	template<typename ReturnValue=Value>boost::iterator_range<const_sparse_iterator<ReturnValue> > nonzeros() const
		{
		return boost::iterator_range<const_sparse_iterator<Value> >(
				const_sparse_iterator<Value>(m_contents.begin()),
				const_sparse_iterator<Value>(m_contents.end()));
		}

	/** Provides a range of boost::index_value<Value,Index> holding the nonzeros.
	 */

	template<typename ReturnValue=Value>boost::iterator_range<const_dense_iterator<ReturnValue> > dense() const
		{
		return boost::iterator_range<const_dense_iterator<Value> >(
				const_dense_iterator<Value>(m_contents.begin(),m_contents.end(),0),
				const_dense_iterator<Value>(m_contents.end(),m_contents.end(),m_N));
		}

	/// Number of nonzero elements
	std::size_t nnz() const { return m_contents.size(); }

	/// Sparse vector dimension (N) regardless of number of zeros
	std::size_t dim() const { return m_N; }

	/// Accessor function returning default-constructed value for missing elements
	const Value& operator[](Index i) const;

private:
//	SparseVector(std::size_t Nmax,boost::container::flat_map<Index,Value>&& m) : m_contents(std::move(m)),m_N(Nmax){}

	boost::container::flat_map<Index,Value> 	m_contents;
	Index 										m_N=0;
};

template<typename Index,typename Value>template<class Iterator>SparseVector<Index,Value>::SparseVector(Iterator begin,const Iterator end,const std::size_t N,const std::size_t Nnz)
		: m_N(N)
{
	m_contents = boost::container::flat_map<Index,Value>(begin,end);
}

template<typename Index,typename Value>const Value& SparseVector<Index,Value>::operator[](Index i) const
{
	const auto it = m_contents.find(i);
	return it == m_contents.end() ? Value() : it->second;
}



#endif /* OUTPUTTYPES_SPARSEVECTOR_HPP_ */
