/*
 * SparseVector.hpp
 *
 *  Created on: Oct 20, 2015
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_SPARSEVECTOR_HPP_
#define OUTPUTTYPES_SPARSEVECTOR_HPP_

#ifndef SWIG_OPENBRACE
#define SWIG_OPENBRACE
#define SWIG_CLOSEBRACE
#endif

SWIG_OPENBRACE

#include <boost/container/flat_map.hpp>
#include <boost/range/algorithm.hpp>
#include <vector>
#include <iostream>

#include <fstream>

#include "SparseVector.hpp"

SWIG_CLOSEBRACE



template<typename Index,typename Value> class SparseVector
{
public:
	SparseVector() {}

	// return a dense representation
	std::vector<Value> dense(std::size_t N) const
	{
		std::vector<Value> v(N,0);
		for(const auto p : contents_)
		{
			assert(p.first < v.size());
			v[p.first] = p.second;
		}
		return v;
	}

	std::size_t nnz() const { return Nnz_; }


	static SparseVector<Index,Value> loadTextFile(const std::string& fn,std::size_t Nmax=0);

private:
	SparseVector(std::size_t Nmax,boost::container::flat_map<Index,Value>&& m) : contents_(std::move(m)),Nmax_(Nmax),Nnz_(countNnz()){}

	std::size_t countNnz() const
	{
		return boost::count_if(contents_, [](std::pair<Index,Value> i){ return i.second != 0; });
	}

	boost::container::flat_map<Index,Value> contents_;
	Index Nmax_=0;
	Index Nnz_=0;

};


#include <string>

template<typename Index,typename Value>SparseVector<Index,Value> SparseVector<Index,Value>::loadTextFile(const std::string& fn,std::size_t Nmax)
{
	std::ifstream is(fn.c_str());

	std::vector<std::pair<Index,Value>> v;

	std::string s;

	if (!is.good())
		throw std::logic_error("Failed to open file");

	// strip leading comments
	for( std::getline(is,s); !is.eof() && (s[0] == '%' || s[0] == '#'); std::getline(is,s))
		std::cout << "INFO: comment line '" << s << "'" << std::endl;

	std::size_t maxIdx=Nmax;

	while(!is.eof())
	{
		Index idx;
		Value x;

		std::stringstream ss(s);
		ss >> idx >> x;

		v.push_back(std::make_pair(idx,x));

		maxIdx=std::max(maxIdx,(std::size_t)idx);

		std::getline(is,s);
	}

	boost::container::flat_map<Index,Value> m(v.begin(),v.end());

	return SparseVector<Index,Value>(maxIdx,std::move(m));
}

#ifdef SWIG

%template(SparseVectorUD) SparseVector<unsigned,double>;

#endif


#endif /* OUTPUTTYPES_SPARSEVECTOR_HPP_ */
