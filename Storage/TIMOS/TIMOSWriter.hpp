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
#include <FullMonte/Geometry/Sources/Visitor.hpp>
#include <FullMonte/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonte/Geometry/Sources/Volume.hpp>
#include <FullMonte/Geometry/Sources/PencilBeam.hpp>
#include <FullMonte/Geometry/Sources/PointSource.hpp>
#include <FullMonte/Geometry/Sources/Composite.hpp>

#include <string>
#include <vector>

#include <iostream>

class TIMOSWriter : public TIMOS
{
public:
	TIMOSWriter(std::string pfx) : TIMOS(pfx){}

	virtual ~TIMOSWriter(){}

	void write(Source::Base*);

	virtual void writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
	virtual void writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;

	class SourceVisitor;			/// Visitor for sources

	const std::string&				comment()				const 	{ return m_comment; }
	void							comment(std::string c)			{ m_comment=c; 		}


private:
	static void writeUserComments(std::ostream&,std::string);

	std::string m_comment;
};

class TIMOSWriter::SourceVisitor : public Source::Visitor
{
public:
	SourceVisitor(std::ostream& os) : m_os(os){}

	void print(Source::Base* b);

	void startVisit(Source::Base* b);

	virtual void visit(Source::PointSource* p) 		override;

	virtual void visit(Source::Volume* v) 		override;

	virtual void visit(Source::SurfaceTri* st) 	override;
	virtual void visit(Source::Composite* c) 	override;
	virtual void visit(Source::PencilBeam* pb) 	override;

	virtual void visit(Source::Ball* b)			override;
	virtual void visit(Source::Line* l)			override;
	virtual void visit(Source::Base* b)			override;
	virtual void visit(Source::Surface* s)		override;

private:
	std::ostream& m_os;
};

#endif /* TIMOSWRITER_HPP_ */
