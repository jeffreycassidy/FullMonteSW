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

	DenseVector(){}

	/// Move from a vector
	DenseVector(std::vector<Value>&& v) : m_v(v)
		{ updateStats(); }

	/// Copy from a vector
	DenseVector(const std::vector<Value>& v) : m_v(v)
		{ updateStats(); }

	/// Construct empty of a given size
	explicit DenseVector(unsigned dim) : m_v(dim,Value()){}


	typedef boost::iterator_range<typename std::vector<Value>::const_iterator> 					const_value_iterator;

	/// Returns the values in sequence
	const_value_iterator values() const { return m_v; }



	/// Class to provide an indexed iterator to the dense vector
	class const_dense_iterator : public boost::iterator_adaptor<
			const_dense_iterator,
			typename std::vector<Value>::const_iterator,
			std::pair<Index,Value>,
			boost::forward_traversal_tag,
			const std::pair<Index,Value>&,
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
			m_val = std::make_pair(m_idx,*base_reference());
		}

		using const_dense_iterator::iterator_adaptor_::base_reference;

		void increment()
		{
			++m_idx;
			++base_reference();

			m_val = std::make_pair(m_idx,*base_reference());
		}

		const std::pair<Index,Value>& dereference() const
		{
			return m_val;
		}

	private:
		Index 										m_idx;
		std::pair<Index,Value>						m_val;
		friend class boost::iterator_core_access;
	};

	/// Returns all values, indexed
	typedef boost::iterator_range<const_dense_iterator>	const_dense_range;
	const_dense_range dense() const
		{ return boost::iterator_range<const_dense_iterator>(
				const_dense_iterator(m_v.begin(),0),
				const_dense_iterator(m_v.end(),  0)); }



	/// Class to iterate through sparsely
	class const_sparse_iterator : public boost::iterator_adaptor<
			const_sparse_iterator,
			typename std::vector<Value>::const_iterator,
			std::pair<Index,Value>,
			boost::forward_traversal_tag,
			const std::pair<Index,Value>&,
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
			while(base_reference() != m_end && !nonzero(*base_reference()))
				increment();
			m_val = std::make_pair(m_idx,*base_reference());
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
			while (base_reference() != m_end && !nonzero(*base_reference()));

			m_val = std::make_pair(m_idx,*base_reference());
		}

		const std::pair<Index,Value>& dereference() const
		{
			return m_val;
		}

	private:
		Index 										m_idx;
		std::pair<Index,Value>						m_val;
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

	Value		sum() const { return m_sum;			}

	Value operator[](unsigned i) const { return m_v[i]; }

private:

	void updateStats()
	{
		m_nnz=0;
		m_sum=Value();
		for(const auto v : m_v)
		{
			m_nnz += nonzero(v);
			m_sum += v;
		}

	}
	Index				m_startIndex=0;
	std::size_t 		m_nnz=0;
	std::vector<Value> 	m_v;
	Value 				m_sum;
};




#endif /* OUTPUTTYPES_DENSEVECTOR_HPP_ */
