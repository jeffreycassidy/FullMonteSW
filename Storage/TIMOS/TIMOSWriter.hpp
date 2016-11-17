/*
 * TIMOSWriter.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#ifndef TIMOSWRITER_HPP_
#define TIMOSWRITER_HPP_

#include "TIMOS.hpp"

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/newgeom.hpp>

#include <FullMonteSW/Geometry/Sources/ForwardDecl.hpp>


#include <string>
#include <vector>

#include <iostream>

class TIMOSWriter : public TIMOS
{
public:
	TIMOSWriter(std::string pfx) : TIMOS(pfx){}

	virtual ~TIMOSWriter(){}

	void write(Source::Abstract*);
	void writeMesh(const TetraMesh& M) const;

	virtual void writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
	virtual void writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;

	class SourceVisitor;			/// Visitor for sources

	const std::string&				comment()				const 	{ return m_comment; }
	void							comment(std::string c)			{ m_comment=c; 		}


private:
	static void writeUserComments(std::ostream&,std::string);

	std::string m_comment;
};



#endif /* TIMOSWRITER_HPP_ */

