/*
 * list_adaptors.hpp
 *
 *  Created on: Feb 16, 2015
 *      Author: jcassidy
 */

#ifndef LIST_ADAPTORS_HPP_
#define LIST_ADAPTORS_HPP_

#include <iterator>
#include <boost/range.hpp>

struct take {
	size_t _n;
	take(size_t n=1) : _n(n){}
};

struct drop {
	size_t _n;
	drop(size_t n=1) : _n(n){}
};

struct init_tag {};
init_tag init;

drop tail(1);
take head(1);

#include <iostream>

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

// same as above, but takes range by value (eg. boost::iterator_range)

template<class Iterator>typename boost::iterator_range<Iterator> operator| (boost::iterator_range<Iterator> r,drop d)
{
	if (boost::size(r) > d._n)
	{
		Iterator new_begin = begin(r);
		new_begin = std::next(new_begin,d._n);
		return boost::iterator_range<Iterator>(new_begin, end(r));
	}
	else
		return boost::iterator_range<Iterator>(end(r), end(r));
}


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

template<class Range>boost::iterator_range<typename boost::range_iterator<Range>::type> operator|(Range& r,init_tag)
{
	typedef typename boost::range_iterator<Range>::type iterator;
	iterator new_end=end(r);
	size_t N=boost::size(r);

	if (N > 1)
		new_end = std::next(begin(r),N-1);

	return boost::iterator_range<iterator>(begin(r),new_end);
}


#endif /* LIST_ADAPTORS_HPP_ */
