/*
 * TextFile.hpp
 *
 *  Created on: Nov 12, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_TEXTFILE_TEXTFILE_HPP_
#define STORAGE_TEXTFILE_TEXTFILE_HPP_

#include "../LineFile.hpp"

#include <string>

template<typename Index,typename Value>SparseVector<Index,Value> SparseVector<Index,Value>::loadTextFile(const std::string& fn,std::size_t Nmax)
{
	std::ifstream is(fn.c_str());

	if (!is.good())
		throw std::logic_error("Failed to open file");


	LineStream ls(is);

	std::vector<std::pair<Index,Value>> v;

	std::size_t maxIdx=Nmax;

	while(const const string s : ls.lines())
	{
		Index idx;
		Value x;

		std::stringstream ss(s);
		ss >> idx >> x;

		v.push_back(std::make_pair(idx,x));

		maxIdx=std::max(maxIdx,(std::size_t)idx);
	}

	boost::container::flat_map<Index,Value> m(v.begin(),v.end());

	return SparseVector<Index,Value>(maxIdx,std::move(m));
}




#endif /* STORAGE_TEXTFILE_TEXTFILE_HPP_ */
