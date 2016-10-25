/*
 * Reader.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_READER_HPP_
#define STORAGE_READER_HPP_

#include <vector>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/Sources/Base.hpp>
#include <FullMonteSW/FullMonte/Kernels/Software/Material.hpp>

class Reader {
public:
	Reader(){}
	virtual TetraMesh 						mesh()			const=0;
	virtual std::vector<Source::Abstract*> 		sources()		const=0;

	virtual ~Reader(){}

	virtual void clear()=0;
};

#endif /* STORAGE_READER_HPP_ */

