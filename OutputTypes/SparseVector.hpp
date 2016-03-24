/*
 * SparseVector.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_SPARSEVECTOR_HPP_
#define OUTPUTTYPES_SPARSEVECTOR_HPP_

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/nvp.hpp>

#include "SparseBase.hpp"

/** Read-only sparse vector, with iteration over either all values (including zeros), or indexed nonzeros.
 *
 * @tparam	Index		The index type.
 * @tparam	Value		The value type. Must be equality-comparable, copyable/assignable, and default-constructible.
 */
struct value_range_t {};
struct indexed_range_t {};
struct ordered_indexed_range_t{};

template<typename ValueType,typename IndexType=unsigned> class SparseVector
{
public:
	typedef ValueType Value;
	typedef IndexType Index;
	SparseVector(){}

	/// Construct from an Iterator range of values (implicitly indexed 0..N-1 -> ordered, unique indices)
	template<class ValueRange>SparseVector(value_range_t,ValueRange r,std::size_t nnzHint=0);

	/// Construct from an IndexedRange of values, not necessarily unique or ordered
	template<class IndexedRange>SparseVector(indexed_range_t,std::size_t dim,IndexedRange ir,std::size_t nnzHint=0);

	/// Construct from an ordered indexed range of values
	template<class IndexedRange>SparseVector(ordered_indexed_range_t,std::size_t dim,IndexedRange ir,std::size_t nnzHint=0);

	SparseVector(SparseVector&& sv) 				= default;
	SparseVector(const SparseVector& sv) 			= default;
	SparseVector& operator=(const SparseVector& sv) = default;



	/** Adapted iterator that returns an indexed value for all elements including zeroes.
	 */

	class const_dense_iterator : public boost::iterator_adaptor<
			const_dense_iterator,
			typename std::vector<std::pair<Index,Value>>::const_iterator,
			std::pair<Index,Value>,
			boost::forward_traversal_tag,
			const std::pair<Index,Value>&,
			std::ptrdiff_t>
	{
	public:
		typedef typename const_dense_iterator::iterator_adaptor_ Super;
		typedef typename Super::base_type base_type;
		using Super::base_reference;

		const_dense_iterator(){}

		const_dense_iterator(const const_dense_iterator&) = default;
		const_dense_iterator(const base_type& it,const base_type& itEnd,Index idx=0) :
			const_dense_iterator::iterator_adaptor_(it),
			m_itEnd(itEnd),
			m_Idx(idx)
		{
			if (base_reference() != m_itEnd)
				m_pair = std::make_pair(m_Idx,base_reference()->second);
			else
				m_pair = std::make_pair(m_Idx,Value());
		}

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

			m_pair = std::make_pair(m_Idx,base_reference() != m_itEnd && base_reference()->first == m_Idx ? base_reference()->second : Value());
		}

		const std::pair<Index,Value>& dereference() const
		{
			return m_pair;
		}

	private:
		const base_type				m_itEnd;
		std::pair<Index,Value>		m_pair;
		Index 						m_Idx=0;
		friend class boost::iterator_core_access;
	};


	/** Provides a range of std::pair<Index,Value> holding the nonzeros.
	 */

	typedef boost::iterator_range<typename std::vector<std::pair<Index,Value>>::const_iterator> const_sparse_range;

	const_sparse_range nonzeros() const
	{
		return const_sparse_range(
				m_contents.begin(),
				m_contents.end());
	}



	/** Provides a range of std::pair<Index,Value> holding the nonzeros.
	 */

	typedef boost::iterator_range<const_dense_iterator> const_dense_range;

	const_dense_range dense() const
	{
		return boost::iterator_range<const_dense_iterator>(
				const_dense_iterator(m_contents.begin(),m_contents.end(),0),
				const_dense_iterator(m_contents.end(),	m_contents.end(),m_dim));
	}


	/** Provides all elements, values only */

	class const_value_iterator : public boost::iterator_adaptor<
		const_value_iterator,
		typename std::vector<std::pair<Index,Value>>::const_iterator,
		Value,
		boost::forward_traversal_tag,
		const Value&,
		std::ptrdiff_t>
	{
	public:
		typedef typename const_value_iterator::iterator_adaptor_ Super;
		typedef typename Super::base_type base_type;
		using Super::base_reference;

		const_value_iterator(){}
		const_value_iterator(const const_value_iterator&) = default;
		const_value_iterator(const base_type& it,unsigned idx,const Value& def) : Super(it),
				m_idx(idx),
				m_default(def){}

		void increment()
		{
			++m_idx;
			if (base_reference()->first < m_idx)
				++base_reference();
		}

		const Value& dereference() const
		{
			if (m_idx == base_reference()->first)
				return this->base_reference()->second;
			else
				return m_default;
		}

	private:
		unsigned 	m_idx;
		Value		m_default;
		friend class boost::iterator_core_access;
	};

	typedef boost::iterator_range<const_value_iterator> const_value_range;

	const_value_range values() const
	{
		return const_value_range(
				const_value_iterator(m_contents.begin(),0,Value()),
				const_value_iterator(m_contents.end(),m_dim,Value()));
	}

	/// Number of nonzero elements
	std::size_t nnz() const { return m_contents.size(); }

	/// Sparse vector dimension (N) regardless of number of zeros
	std::size_t dim() const { return m_dim; }

	/// Accessor function returning default-constructed value for missing elements (uses binary search)
	Value operator[](Index i) const;

	Value sum() const
	{
		return m_sum;
	}

private:
	void construct()
	{
		// shrink if needed
		if (m_contents.size() < m_contents.capacity())
			std::vector<std::pair<Index,Value>>(m_contents).swap(m_contents);

		// gather sum
		m_sum=Value();
		for(const auto o : m_contents)
			m_sum += o.second;
	}

	static bool compareFirst(const std::pair<Index,Value>& lhs,const std::pair<Index,Value>& rhs){ return lhs.first < rhs.first; }


	std::vector<std::pair<Index,Value>> 	m_contents;
	Index 									m_dim=0;
	Value									m_sum=Value();

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
	{
		ar & boost::serialization::make_nvp("dim",m_dim)
			& boost::serialization::make_nvp("values",m_contents);
		if (Archive::is_loading::value)
			construct();
	}
	friend class boost::serialization::access;
};

template<typename Value,typename Index>Value SparseVector<Value,Index>::operator[](Index i) const
{
	const auto it = boost::lower_bound(m_contents, std::make_pair(i,Value()), compareFirst);

	if (it == m_contents.end() || it->first != i)
		return Value();
	else
		return it->second;
}

template<typename Value,typename Index>template<class IndexedRange>SparseVector<Value,Index>::SparseVector(indexed_range_t,std::size_t dim,IndexedRange ir,std::size_t nnzHint) :
		m_dim(dim)
{
	m_contents.reserve(nnzHint);

	for(const auto iv : ir)
		if (iv.value() != 0)
			m_contents.push_back(std::make_pair(index(iv),value(iv)));

	boost::sort(m_contents,compareFirst);

	typename std::vector<std::pair<Index,Value>>::const_iterator i=m_contents.begin();
	typename std::vector<std::pair<Index,Value>>::iterator o=m_contents.begin();

	++i;

	for(; i != m_contents.end(); ++i)
	{
		if (i->first == o->first)
			o->second += i->second;
		else if (i != o)
			*(++o) = *i;
	}

	m_contents.resize(o-m_contents.begin());

	construct();
}

template<typename Value,typename Index>template<class IndexedRange>SparseVector<Value,Index>::SparseVector(ordered_indexed_range_t,std::size_t dim,IndexedRange ir,std::size_t nnzHint) :
		m_dim(dim)
{
	m_contents.reserve(nnzHint);

	for(const auto iv : ir)
		if (value(iv) != 0)
			m_contents.push_back(std::make_pair(index(iv),value(iv)));

	construct();
}

template<typename Value,typename Index>template<class ValueRange>SparseVector<Value,Index>::SparseVector(value_range_t,ValueRange R,const std::size_t nnzHint) :
		m_dim(0)
{
	m_contents.reserve(nnzHint);

	for(auto it=begin(R); it != end(R); ++it,++m_dim)
		if (nonzero(*it))
			m_contents.push_back(std::make_pair(m_dim,*it));

	construct();
}

#endif /* OUTPUTTYPES_SPARSEVECTOR_HPP_ */
