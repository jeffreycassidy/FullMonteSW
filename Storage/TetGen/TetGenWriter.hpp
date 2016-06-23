/*
 * TetGenWriter.hpp
 *
 *  Created on: May 16, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_TETGEN_TETGENWRITER_HPP_
#define STORAGE_TETGEN_TETGENWRITER_HPP_

#include "TetGenBase.hpp"
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/indexed.hpp>

namespace TetGen {

class TetGenWriter : public TetGenBase {
public:

private:
	template<class PointRange> void write_node_file(std::string,const PointRange&);
	template<class TetRange>   void write_ele_file(std::string,const TetRange&);

};

// <#points> 3 <#attributes> <#boundary marker 0/1>

template<class PointRange>void TetGenWriter::write_node_file(const std::string fn,const PointRange& P)
{
	std::ofstream os(fn.c_str());

	os << boost::size(P) << " 3 0 0" << std::endl;

	for(const auto& p : P | boost::adaptors::indexed(0U))
		os << p.index() << ' ' << p.value()[0] << ' ' << p.value()[1] << ' ' << p.value()[2] << std::endl;

}


// TetRange must dereference to pair<array<int_idp_t,4>, int_region_t>

template<class TetRange>void TetGenWriter::write_ele_file(const std::string fn,const TetRange& T)
{
	std::ofstream os(fn.c_str());

	os << boost::size(T) << " 4 1" << std::endl;

	for(const auto& t : T | boost::adaptors::indexed(0U))
		os << t.index() << ' ' << t.value().first[0] << ' ' << t.value().first[1] << ' ' << t.value().first[2]
		      << ' ' << t.value().first[3] << ' ' << t.value().second << std::endl;
}


}





#endif /* STORAGE_TETGEN_TETGENWRITER_HPP_ */
