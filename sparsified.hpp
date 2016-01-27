/*
 * sparsified.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef SPARSIFIED_HPP_
#define SPARSIFIED_HPP_

#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/filtered.hpp>


/** Makes a range sparse according to some threshold and comparison function
 *
 * R | sparsified(Index i0,Value th,Comp cmp)
 *
 * Will return a boost::range::index_value<Value,Index> containing (i,R[i]) for each element of R where cmp(r[i], th) == False
 *
 * Watch out for argument type to sparsified; must match _exactly_ on index & value types (eg. 0 vs 0U, 0L, 0LL, 0f, etc)
 */

template<class Value,class Comp=std::less<Value>>struct sparsifier
{
	sparsifier(std::ptrdiff_t index,Value threshold,Comp comp=std::less<Value>()) : index_(index),threshold_(threshold),comp_(comp){}
	std::ptrdiff_t index_;
	Value threshold_;
	Comp comp_;

	bool operator()(const boost::range::index_value<const Value&,std::ptrdiff_t>& iv) const { return comp_(threshold_,iv.value()); }
};

template<class Value,class Comp=std::less<Value>>sparsifier<Value,Comp> sparsified(std::ptrdiff_t i0,const Value threshold,const Comp comp=Comp())
{
	return sparsifier<Value,Comp>(i0,threshold,comp);
}

template<class Range,class Value,class Comp>auto operator|(const Range& R,sparsifier<Value,Comp> S) ->
		decltype(R | boost::adaptors::indexed(S.index_) | boost::adaptors::filtered(S))
{
	return R |
			boost::adaptors::indexed(S.index_) |
			boost::adaptors::filtered(S);
}


#endif /* SPARSIFIED_HPP_ */
