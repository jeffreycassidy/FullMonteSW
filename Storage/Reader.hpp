/*
 * Reader.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_READER_HPP_
#define STORAGE_READER_HPP_

#include <vector>
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>

class SourceDescription;

class Reader {
public:
	virtual TetraMesh 						mesh()			const=0;
	virtual std::vector<Material> 			materials() 	const=0;
	virtual std::vector<SourceDescription*> sources()		const=0;

	virtual ~Reader(){}

	virtual void clear()=0;
};

#endif /* STORAGE_READER_HPP_ */
