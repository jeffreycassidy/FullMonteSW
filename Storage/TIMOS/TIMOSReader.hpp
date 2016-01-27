/*
 * TIMOS.hpp
 *
 *  Created on: Mar 5, 2015
 *      Author: jcassidy
 */

#ifndef TIMOSREADER_HPP_
#define TIMOSREADER_HPP_

#include "TIMOS.hpp"
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>
#include <unordered_map>


class SuperSimple
{
public:
	SuperSimple()
{
}
};

class TIMOSReader : public TIMOS {
public:
	TIMOSReader() {}
	TIMOSReader(std::string pfx) : TIMOS(pfx){}

	virtual ~TIMOSReader(){}

	virtual TetraMesh						mesh();
	virtual std::vector<Material>			materials();
	virtual std::vector<Source::Base*>		sources();

	virtual std::vector<SimpleMaterial>		materials_simple();

	virtual std::vector<LegendEntry> 		legend();

	virtual void clear() {};

private:

	virtual Optical 					parse_optical(std::string fn)=0;
	virtual std::vector<SourceDef>		parse_sources(std::string fn)=0;
	virtual Mesh						parse_mesh(std::string fn) 	=0;
	virtual std::vector<LegendEntry> 	parse_legend(std::string fn)=0;
};



#endif /* TIMOS_HPP_ */
