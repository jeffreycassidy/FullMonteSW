/*
 * densified.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef DENSIFIED_HPP_
#define DENSIFIED_HPP_


/** Densifies a sparse range (ie a range that dereferences to boost::range::index_value<Value,Index>)
 *
 * R | densified(i0,N,v0)
 *
 * Will iterate through N elements [i0,i0+N), with values being taken from the sparse range R where present and provided as v0 else.
 * The indices of the sparse range must be unique and in increasing order.
 */

template<class Iterator,class Index,class Value>class densify_iterator : public boost::iterator_facade<
	densify_iterator<Iterator,Index,Value>,
	const Value,
	boost::bidirectional_traversal_tag,
	const Value&,
	std::ptrdiff_t>
{

public:

	densify_iterator(Index i0,Value v0,Iterator sparseBegin,Iterator sparseEnd) : i_(i0),v0_(v0),it_(sparseBegin),itEnd_(sparseEnd){}

	void increment()
	{
		if (it_ != itEnd_ && i_ == it_->index())
			++it_;
		++i_;
	}

	bool equal(const densify_iterator& rhs) const
	{
		return i_ == rhs.i_;
	}

	const Value& dereference() const
	{
		if (it_ == itEnd_ || it_->index() != i_)
			return v0_;
		else
			return it_->value();
	}

private:
	Index i_;
	Value v0_;
	Iterator it_;
	Iterator itEnd_;
};



template<class Value>class densify_placeholder
{
public:
	densify_placeholder(std::ptrdiff_t i0,std::ptrdiff_t N,Value v0=Value()) : i0_(i0),N_(N),v0_(v0){}

	std::ptrdiff_t i0_;
	std::ptrdiff_t N_;
	Value v0_;
};

template<class Value>densify_placeholder<Value> densified(std::ptrdiff_t i0,std::ptrdiff_t N,Value v0)
{
	return densify_placeholder<Value>(i0,N,v0);
}


template<class Range,class Value>boost::iterator_range<densify_iterator<typename boost::range_iterator<Range>::type,std::ptrdiff_t,Value>> operator|(Range& R,const densify_placeholder<Value> d)
{
	return boost::iterator_range<densify_iterator<typename boost::range_iterator<Range>::type,std::ptrdiff_t,Value>>(
			densify_iterator<typename boost::range_iterator<Range>::type,std::ptrdiff_t,Value>(d.i0_,d.v0_,begin(R),end(R)),
			densify_iterator<typename boost::range_iterator<Range>::type,std::ptrdiff_t,Value>(d.i0_+d.N_,d.v0_,end(R),end(R)));
}




#endif /* DENSIFIED_HPP_ */
