/*
 * take_drop.hpp
 *
 *  Created on: Feb 16, 2015
 *      Author: jcassidy
 */

#ifndef LIST_ADAPTORS_HPP_
#define LIST_ADAPTORS_HPP_

#include <iterator>
#include <boost/range.hpp>

/** S | take(N) 		Takes the first N elements of a sequence.
 *
 */

struct take {
	size_t _n;
	take(size_t n=1) : _n(n){}
};

template<class Range>boost::iterator_range<typename boost::range_iterator<Range>::type> operator| (Range& r,take t)
{
	typedef typename boost::range_iterator<Range>::type iterator;

	if (boost::size(r) > t._n)
	{
		iterator new_end = begin(r);
		new_end = std::next(new_end,t._n);
		return boost::iterator_range<iterator>(begin(r), new_end);
	}
	else
		return boost::iterator_range<iterator>(begin(r), end(r));
}




/** S | drop(N)			Drops the first N elements of a sequence, returning the remainder
 *
 */

struct drop {
	size_t _n;
	drop(size_t n=1) : _n(n){}
};

template<class Range>boost::iterator_range<typename boost::range_iterator<Range>::type> operator| (Range& r,drop d)
{
	typedef typename boost::range_iterator<Range>::type iterator;

	if (boost::size(r) > d._n)
	{
		iterator new_begin = begin(r);
		new_begin = std::next(new_begin,d._n);
		return boost::iterator_range<iterator>(new_begin, end(r));
	}
	else
		return boost::iterator_range<iterator>(end(r), end(r));
}



// 			v[0] v[1] ... v[n-2] v[n-1]
// head		____
// tail			 ______________________
//
// init     ____________________
// last							  _____

drop tail(1);
take head(1);



/** S | init		Takes all but the last element of a sequence
 *
 */

struct init_tag {};
init_tag init;

template<class Range>boost::iterator_range<typename boost::range_iterator<Range>::type> operator|(Range& r,init_tag)
{
	typedef typename boost::range_iterator<Range>::type iterator;
	iterator new_end=end(r);

	if (new_end != begin(r))
		new_end--;

	return boost::iterator_range<iterator>(begin(r),new_end);
}


#endif /* TAKE_DROP_HPP_ */
