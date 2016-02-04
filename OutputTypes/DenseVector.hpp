/*
 * DenseVector.hpp
 *
 *  Created on: Nov 13, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_DENSEVECTOR_HPP_
#define OUTPUTTYPES_DENSEVECTOR_HPP_

#include <boost/iterator/iterator_adaptor.hpp>

#include <vector>
#include <boost/range.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <iostream>

template<typename ValueType,typename IndexType=unsigned>class DenseVector
{
public:
	typedef ValueType Value;
	typedef IndexType Index;

private:
	static bool value_nonzero(boost::range::index_value<const Value&,Index> iv)
			{ return iv.value() != 0; }

public:
	DenseVector(){}

	/// Move from a vector
	DenseVector(std::vector<Value>&& v) : m_v(v)
		{ countNonZero(); }

	/// Copy from a vector
	DenseVector(const std::vector<Value>& v) : m_v(v)
		{ countNonZero(); }

	/// Construct empty of a given size
	explicit DenseVector(unsigned dim) : m_v(dim,Value()){}

	typedef boost::iterator_range<typename std::vector<Value>::const_iterator> 					const_value_iterator;
	//typedef decltype(std::declval<std::vector<Value>>() | boost::adaptors::indexed(Index()) | boost::adaptors::transformed(byValue())) 	const_dense_range;

	/// Returns the values in sequence
	const_value_iterator values() const { return m_v; }

	class const_dense_iterator : public boost::iterator_adaptor<
			const_dense_iterator,
			typename std::vector<Value>::const_iterator,
			std::pair<Index,Value>,
			boost::forward_traversal_tag,
			std::pair<Index,Value>,
			std::ptrdiff_t>
	{
	public:

		typedef typename const_dense_iterator::iterator_adaptor_ Super;

		const_dense_iterator(){}
		const_dense_iterator(const const_dense_iterator&) = default;
		const_dense_iterator(const typename std::vector<Value>::const_iterator& it,Index idx) :
			Super(it),
			m_idx(idx)
		{
			m_val = *base_reference();
		}

		using const_dense_iterator::iterator_adaptor_::base_reference;

		void increment()
		{
			++m_idx;
			++base_reference();

			m_val = *base_reference();
		}

		std::pair<Index,Value> dereference() const
		{
			return std::make_pair(m_idx,m_val);
		}

	private:
		Index 										m_idx;
		Value										m_val;
		friend class boost::iterator_core_access;
	};

	/// Returns all values, indexed
	typedef boost::iterator_range<const_dense_iterator>	const_dense_range;
	const_dense_range dense() const
		{ return boost::iterator_range<const_dense_iterator>(
				const_dense_iterator(m_v.begin(),0),
				const_dense_iterator(m_v.end(),  0)); }


	class const_sparse_iterator : public boost::iterator_adaptor<
			const_sparse_iterator,
			typename std::vector<Value>::const_iterator,
			std::pair<Index,Value>,
			boost::forward_traversal_tag,
			std::pair<Index,Value>,
			std::ptrdiff_t>
	{
	public:
		const_sparse_iterator(){}
		const_sparse_iterator(const const_sparse_iterator&) = default;
		const_sparse_iterator(const typename std::vector<Value>::const_iterator& it,const typename std::vector<Value>::const_iterator& itEnd) :
			const_sparse_iterator::iterator_adaptor_(it),
			m_idx(0),
			m_end(itEnd)
		{
			while(base_reference() != m_end && *base_reference() == 0)
				increment();
		}

		using const_sparse_iterator::iterator_adaptor_::base_reference;

		void increment()
		{
			if (base_reference() == m_end)
				return;

			do
			{
				++base_reference();
				++m_idx;
			}
			while (base_reference() != m_end && *base_reference() == 0);
		}

		std::pair<Index,Value> dereference() const
		{
			return std::make_pair(m_idx,*base_reference());
		}

	private:
		Index 										m_idx;
		typename std::vector<Value>::const_iterator m_end;
		friend class boost::iterator_core_access;
	};

	typedef boost::iterator_range<const_sparse_iterator> const_sparse_range;

	/// Returns the nonzero values, indexed
	const_sparse_range nonzeros() const
		{ return const_sparse_range(
				const_sparse_iterator(m_v.begin(),m_v.end()),
				const_sparse_iterator(m_v.end(),m_v.end())); }

	std::size_t nnz() const { return m_nnz; 		}
	std::size_t dim() const { return m_v.size(); 	}

	Value operator[](unsigned i) const { return m_v[i]; }

private:

	void countNonZero()
	{
		m_nnz = boost::size(nonzeros());
	}
	Index			m_startIndex=0;
	std::size_t 	m_nnz=0;
	std::vector<Value> 	m_v;
};




#endif /* OUTPUTTYPES_DENSEVECTOR_HPP_ */
