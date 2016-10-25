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
#include <FullMonteSW/Geometry/Sources/Abstract.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>
#include <FullMonteSW/Geometry/Sources/PointSource.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>

#include <string>
#include <vector>

#include <iostream>

class TIMOSWriter : public TIMOS
{
public:
	TIMOSWriter(std::string pfx) : TIMOS(pfx){}

	virtual ~TIMOSWriter(){}

	void write(Source::Abstract*);

	virtual void writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
	virtual void writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;

	class SourceVisitor;			/// Visitor for sources

	const std::string&				comment()				const 	{ return m_comment; }
	void							comment(std::string c)			{ m_comment=c; 		}


private:
	static void writeUserComments(std::ostream&,std::string);

	std::string m_comment;
};

#ifndef SWIG

class TIMOSWriter::SourceVisitor : public Source::Abstract::Visitor
{
public:
	SourceVisitor(std::ostream& os) : m_os(os){}

	void print(Source::Abstract* b);

	void visit(Source::Abstract* b);

	virtual void doVisit(Source::Point* p) 		override;

	virtual void doVisit(Source::Volume* v) 		override;

	virtual void doVisit(Source::SurfaceTri* st) 	override;
	virtual void doVisit(Source::Composite* c) 	override;
	virtual void doVisit(Source::PencilBeam* pb) 	override;

	virtual void doVisit(Source::Ball* b)			override;
	virtual void doVisit(Source::Line* l)			override;
	virtual void doVisit(Source::Abstract* b)			override;
	virtual void doVisit(Source::Surface* s)		override;

private:
	std::ostream& m_os;
};

#endif

#endif /* TIMOSWRITER_HPP_ */

