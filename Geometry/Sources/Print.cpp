/*
 * Print.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include "Visitor.hpp"

#include <iostream>
#include <iomanip>
#include <array>

#include "Ball.hpp"
#include "Base.hpp"
#include "PointSource.hpp"
#include "PencilBeam.hpp"
#include "Composite.hpp"
#include "Surface.hpp"
#include "SurfaceTri.hpp"
#include "Volume.hpp"
#include "Line.hpp"

using namespace std;

namespace Source
{

class Printer : public Visitor
{
public:
	Printer(std::ostream& os);
	~Printer();

	virtual void visit(PointSource* p) 	override;
	virtual void visit(Ball* b)			override;
	virtual void visit(Line* l)			override;
	virtual void visit(Volume* v)		override;
	virtual void visit(Composite* c)	override;
	virtual void visit(Surface* s)		override;
	virtual void visit(SurfaceTri* st)	override;
	virtual void visit(Base* b)			override;
	virtual void visit(PencilBeam* b)	override;

private:

	void printPosition(std::array<float,3>);
	void printPower(float);

	std::ostream& m_os;

	unsigned m_coordPrecision=2;
	unsigned m_weightPrecision=0;
	unsigned m_weightWidth=9;
	unsigned m_idWidth=7;
	unsigned m_coordWidth=0;

	unsigned m_prec;
	std::ostream::fmtflags m_fmt;
};

Printer::Printer(std::ostream& os) :
		m_os(os),
		m_prec(os.precision()),
		m_fmt(os.flags())
{
}

Printer::~Printer()
{
	m_os.precision(m_prec);
	m_os.flags(m_fmt);
}

void Printer::printPosition(array<float,3> pos)
{
	m_os << '(' << fixed << setprecision(m_coordPrecision);
	for(unsigned i=0;i<3;++i)
		m_os << setw(m_coordWidth) << pos[i] << (i==2 ? ')' : ',');
}

void Printer::printPower(float w)
{
	m_os << "power " << setprecision(m_weightPrecision) << setw(m_weightWidth) << w;
}

void Printer::visit(PointSource* p)
{
	std::array<float,3> pos = p->position();

	m_os << "Point source ";
	printPosition(p->position());
	m_os << ' ';
	printPower(p->power());
}

void Printer::visit(Ball* p)
{
	m_os << "Ball source ";
	printPosition(p->position());
	m_os << " radius " << setprecision(m_coordPrecision) << setw(m_coordWidth) << p->radius() << ' ';
	printPower(p->power());
}

void Printer::visit(Line* l)
{
	m_os << "Line source ";
	printPosition(l->endpoint(0));
	m_os << " - ";
	printPosition(l->endpoint(1));
	m_os << " length " << setprecision(m_coordPrecision) << setw(m_coordWidth) << l->length() << ' ';
	printPower(l->power());
}

void Printer::visit(Volume* v)
{
	m_os << "Volume source in element " << setw(m_idWidth) << v->elementID() << ' ';
	printPower(v->power());
}

void Printer::visit(Composite* c)
{
	m_os << "Composite source (" << c->count() << " subsources) ";
	printPower(c->power());
}

void Printer::visit(Base* b)
{
	m_os << "<unspecified type> ";
	printPower(b->power());
}

void Printer::visit(Surface* s)
{
	m_os << "Surface patch " << setw(m_idWidth) << s->surfaceID() << ' ';
	printPower(s->power());
}

void Printer::visit(SurfaceTri* st)
{
	m_os << "Surface triangle [";
	std::array<unsigned,3> tri = st->triPointIDs();
	for(unsigned i=0;i<3;++i)
		m_os << setw(m_idWidth) << tri[i] << (i==2 ? '\0' : ' ');
	m_os << "] patch ID " << setw(m_idWidth);
	if (st->surfaceID() == -1U)
		m_os << "???";
	else
		m_os << st->surfaceID();
	m_os << ' ';
	printPower(st->power());
}

void Printer::visit(PencilBeam* pb)
{
	m_os << "Pencil beam position ";
	printPosition(pb->position());
	m_os << " direction ";
	printPosition(pb->direction());
	m_os << " in element " << setw(m_idWidth);
	if (pb->elementHint() == -1U)
		m_os << "???";
	else
		m_os << pb->elementHint();
	m_os << ' ';
	printPower(pb->power());
}

std::ostream& operator<<(std::ostream& os,const Source::Base& b)
{
	Source::Printer P(os);
	((Source::Base&)(b)).acceptVisitor(&P);
	return os;
}

}


