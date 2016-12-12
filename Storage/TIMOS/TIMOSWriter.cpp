/*
 * TIMOSWriter.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#include <iostream>
#include "TIMOS.hpp"
#include "TIMOSWriter.hpp"

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include "../Matlab/take_drop.hpp"

#include <FullMonteSW/Geometry/Sources/Abstract.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/Surface.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/Line.hpp>
#include <FullMonteSW/Geometry/Sources/Point.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>

#include <sstream>

#include <fstream>
#include <iomanip>

struct delim {
	std::string pre,delim,post;
};

struct delim_stream {
	delim D;
	std::ostream& os;
};

delim_stream operator<<(std::ostream& os,delim D)
{
	return delim_stream{ D, os };
}

class TIMOSWriter::SourceVisitor : public Source::Abstract::Visitor
{
public:
	SourceVisitor(std::ostream& os) : m_os(os){}

	void print(Source::Abstract* b);

	virtual void doVisit(Source::Point* p) 		override;

	virtual void doVisit(Source::Volume* v) 		override;

	virtual void doVisit(Source::SurfaceTri* st) 	override;
	virtual void doVisit(Source::PencilBeam* pb) 	override;

	virtual void preVisitComposite(Source::Composite*) override;

private:
	std::ostream& m_os;
};

template<typename T,size_t N>std::ostream& operator<<(const delim_stream ds,const std::array<T,N> a)
{
	auto w = ds.os.width();
	ds.os.width(0);
	ds.os << ds.D.pre;
	ds.os << std::setw(w) << a[0];
	for(unsigned i=1;i<N;++i)
		ds.os << ds.D.delim << std::setw(w) << a[i];
	ds.os << ds.D.post;
	return ds.os;
}

using namespace std;

void TIMOSWriter::writeMesh(const TetraMesh& M) const
{
    ofstream os(meshFn_.c_str());

    if(!os.good())
    {
    	cout << "ERROR - TIMOSWriter::writeMesh(const TetraMesh&) failed to open file '" << meshFn_ << "' for writing" << endl;
    	return;
    }

    os << boost::size(M.points())-1 << endl << M.getNt() << endl;

    for(const auto p : M.points() | drop(1))
    	os << delim{""," ",""} << get(point_coords,M,p) << endl;

    for(const auto t : M.tetras() | drop(1))
    {
    	os << delim{""," ",""} << get(points,M,t) << endl;
    }

    if (os.fail())
    {
    	cout << "ERROR - TIMOSWriter::writeMesh(const TetraMesh&) failure during writing of file '" << meshFn_ << "'" << endl;
    	return;
    }
}

//// TODO: factor this into Material -> TIMOS::Optical conversion followed by writing
//
//void TIMOSWriter::write(const std::vector<Material>& mat) const
//{
//	ofstream os(optFn_.c_str());
//
//	if(!os.good())
//		throw open_for_write_exception("TIMOSWriter::write(std::vector<Material>&) failed to open file for writing");
//
//	os << '1' << endl;
//	os << boost::size(mat)-1 << endl;
//
//	for(Material m : mat | drop(1))
//		os << m.getMuA() << ' ' << m.getMuS() << ' ' << m.getg() << ' ' << m.getn() << endl;
//
//	os << '1' << endl;
//	os << "1.0" << endl;
//
//	if (os.fail())
//		throw write_exception("TIMOSWriter::write(std::vector<Material>&) writing failed");
//}

//void TIMOSWriter::SourceVisitor::visit(Source::Abstract* b)
//{
//	const auto f = m_os.flags();
//	m_os << setprecision(4) << fixed << right;
//	b->acceptVisitor(this);
//	m_os.flags(f);
//}

void TIMOSWriter::SourceVisitor::doVisit(Source::Point* p)
{
	std::array<float,3> pos = p->position();
	m_os << std::setw(2) << TIMOS::SourceDef::Point << ' ' <<
			std::setw(9) << pos[0] << ' ' << std::setw(9) << pos[1] << ' ' << std::setw(9) << pos[2] << ' ' <<
			std::setw(10) << (unsigned long long)(p->power()) << endl;
}

void TIMOSWriter::SourceVisitor::doVisit(Source::Volume* v)
{
	m_os << std::setw(2) << TIMOS::SourceDef::Volume << ' ' << std::setw(7) << v->elementID() << ' ' <<
			std::setw(10) << (unsigned long long)(v->power()) << std::endl;
}

void TIMOSWriter::SourceVisitor::doVisit(Source::SurfaceTri* st)
{
	std::array<unsigned,3> f = st->triPointIDs();
	m_os << std::setw(2) << TIMOS::SourceDef::Face << ' ' <<
			std::setw(7) << f[0] << ' ' <<
			std::setw(7) << f[1] << ' ' <<
			std::setw(7) << f[2] << ' ' <<
			std::setw(10) << (unsigned long long)(st->power()) << std::endl;
}


void TIMOSWriter::SourceVisitor::preVisitComposite(Source::Composite* C)
{
	m_os << C->count() << endl;
}


void TIMOSWriter::SourceVisitor::doVisit(Source::PencilBeam* pb)
{
	std::array<float,3> pos = pb->position();
	std::array<float,3> dir = pb->direction();

	m_os << setw(2) << TIMOS::SourceDef::PencilBeam << ' ' << pb->elementHint() << ' ' <<
		std::setw(9) << pos[0] << ' ' << std::setw(9) << pos[1] << ' ' << std::setw(9) << pos[2] << ' ' <<
		std::setw(7) << dir[0] << ' ' << std::setw(7) << dir[1] << ' ' << std::setw(7) << dir[2] << ' ' <<
		std::setw(10) << (unsigned long long)(pb->power()) << endl;
}
//
//void TIMOSWriter::SourceVisitor::doVisit(Source::Ball* b)
//{}
//void TIMOSWriter::SourceVisitor::doVisit(Source::Line* l)
//{}
//void TIMOSWriter::SourceVisitor::doVisit(Source::Abstract* b)
//{}
//void TIMOSWriter::SourceVisitor::doVisit(Source::Surface* s)
//{}


void TIMOSWriter::write(Source::Abstract* b)
{
	ostream* os;

	if (optFn_ == "")
		os = &cout;
	else
		os = new std::ofstream(sourceFn_.c_str());

	writeUserComments(*os,m_comment);

	SourceVisitor SV(*os);
	SV.visit(b);

	if (os != &cout)
		delete os;
}

// Writes out the volume fluence given an input vector of volume fluence

void TIMOSWriter::writeUserComments(ostream& os,const std::string comm)
{
	char c = os.fill('#');

	os << setw(80) << '#' << endl;
	os << setw(80) << left << "# user comments     " << endl;
	os << '#' << endl;

	std::stringstream ss(comm);
	string s;
	while(!ss.eof())
	{
		std::getline(ss,s);
		os << "# " << s << endl;
	}

	os << '#' << endl;

	os.fill(c);
}

void TIMOSWriter::writeVolFluence(const std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_v,const std::string comm) const
{
	std::ofstream os(fn.c_str());

	writeUserComments(os,comm);

	os << "# file format" << endl;
	os << "# first line: 1 <#tetras> 1" << endl;
	os << "# remaining lines (#tetras repetitions): <IDp0> <IDp1> <IDp2> <IDp3> <volume> <fluence>" << endl;

	assert(phi_v[0] == 0.0);

	size_t nnz = boost::size(phi_v | boost::adaptors::filtered([](double phi){ return phi > 0; }));

	os << "1 " << nnz << " 1" << endl;

	os.fill(' ');

	for(unsigned i=1; i<=mesh.getNt(); ++i)
		if (phi_v[i] > 0.0)
			os << setw(7) << delim{""," ",""} << mesh.getTetraPointIDs(i) <<
					fixed << setprecision(4) << setw(8) << get(volume,mesh,TetraMeshBase::TetraDescriptor(i)) << ' ' <<
					scientific << setprecision(5) << setw(8) << phi_v[i] << endl;
}


// Writes out the surface fluence given an input vector of surface fluence

void TIMOSWriter::writeSurfFluence(const std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,const std::string comm) const
{
	std::ofstream os(fn.c_str());

	os << "# part of FullMonte" << endl;

	writeUserComments(os,comm);

	os << "# file format" << endl;
	os << "# first line: 1 <#faces> 1" << endl;
	os << "# remaining lines (#faces repetitions): <IDp0> <IDp1> <IDp2> <area> <emittance>" << endl;


	std::size_t nnz = boost::size(phi_s | boost::adaptors::filtered([](double x) { return x != 0.0; }));

	os << "1 " << nnz << " 1" << endl;

	os.fill(' ');


	for(const auto& phi : phi_s | boost::adaptors::indexed(0U))
		if (phi.value() > 0.0)

			os << setw(7) << delim{""," ",""} << mesh.getFacePointIDs(phi.index()) <<
					fixed << setprecision(4) << setw(8) << mesh.getFaceArea((int)phi.index()) << ' ' <<
					scientific << setprecision(5) << setw(8) << phi.value() << endl;
}
