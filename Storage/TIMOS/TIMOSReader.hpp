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

	virtual TetraMesh						mesh() const;
	virtual std::vector<Material>			materials() const;
	virtual Source::Base*					sources() const;

	virtual std::vector<SimpleMaterial>		materials_simple() const;

	virtual std::vector<LegendEntry> 		legend() const;

	virtual void clear() {};

private:

	virtual Optical 					parse_optical(std::string fn) const=0;
	virtual std::vector<SourceDef>		parse_sources(std::string fn) const=0;
	virtual Mesh						parse_mesh(std::string fn)  const	=0;
	virtual std::vector<LegendEntry> 	parse_legend(std::string fn) const=0;
};



#endif /* TIMOS_HPP_ */
