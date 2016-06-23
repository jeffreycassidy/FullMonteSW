/*
 * BasisIO.hpp
 *
 *  Created on: Mar 7, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_BASISIO_HPP_
#define GEOMETRY_BASISIO_HPP_

#include <FullMonteSW/Geometry/Basis.hpp>
#include <iostream>
#include <iomanip>

template<typename FT,std::size_t D>inline std::ostream& operator<<(std::ostream& os,const std::array<FT,D> a)
{
	const auto fmt=os.flags();

	unsigned w = os.width();

	os << std::setw(w) << a[0];
	for(unsigned i=1;i<D;++i)
		os << ' ' << std::setw(w) << a[i];

	os.flags(fmt);

	return os;
}

inline std::ostream& operator<<(std::ostream& os,const Basis& b)
{
	const auto fmt = os.flags();

	unsigned w = os.width();

	for(unsigned i=0;i<3;++i)
	{
		os << "Basis vector [" << i << "]: ";
		Vector3 v = b.vector(i);
		for(unsigned j=0;j<3;++j)
			os << std::setw(w) << v[j] << ' ';
		os << std::endl;

	}

	os << "Origin: ";
	Vector3 O = b.vector(3);
	for(unsigned i=0;i<3;++i)
		os << std::setw(w) << O[i] << ' ';
	os << std::endl;

	os.flags(fmt);

	return os;
}




#endif /* GEOMETRY_BASISIO_HPP_ */
