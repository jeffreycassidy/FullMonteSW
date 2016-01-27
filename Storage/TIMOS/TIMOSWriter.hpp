/*
 * TIMOSWriter.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#ifndef TIMOSWRITER_HPP_
#define TIMOSWRITER_HPP_

#include "TIMOS.hpp"

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/newgeom.hpp>
#include <FullMonte/Geometry/Material.hpp>

#include <string>
#include <vector>

class TIMOSWriter : public TIMOS
{
public:
	TIMOSWriter(std::string pfx) : TIMOS(pfx){}

	virtual ~TIMOSWriter(){}

//	virtual void write(const TetraMesh&) const override{}
//	virtual void write(const std::vector<Source::Base*>&) const override{}
//	virtual void write(const std::vector<Material>&) const override{}
//
	virtual void writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
	virtual void writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;


private:
	static void writeUserComments(std::ostream&,std::string);
};

#endif /* TIMOSWRITER_HPP_ */
