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

#include <sstream>

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
//
//void TIMOSWriter::write(const TetraMesh& M) const
//{
//    ofstream os(meshFn_.c_str());
//
//    if(!os.good())
//    	throw open_for_write_exception("TIMOSWriter::write(TetraMeshBase&) failed to open file for writing");
//
//    os << boost::size(M.points())-1 << endl << boost::size(M.tetrasByID())-1 << endl;
//
//    for(Point<3,double> p : M.points() | drop(1))
//    	os << delim{""," ",""} << p << endl;
//
//    for(TetraByPointID t : M.tetrasByID() | drop(1))
//    	os << delim{""," ",""} << t << endl;
//
//    if (os.fail())
//    	throw write_exception("TIMOSWriter::write(TetraMeshBase&) writing failed");
//}
//
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
//
//#define IS_A(p,t,s) const t* p = dynamic_cast<const t*>(s)
//
//void TIMOSWriter::write(const vector<SourceDescription*>& src) const
//{
//	ofstream os(sourceFn_.c_str());
//
//	if(!os.good())
//		throw open_for_write_exception("TIMOSWriter::write(std::vector<SourceDescription>&) failed to open file for writing");
//
//	os << boost::size(src) << endl;
//	for(const SourceDescription* s : src)
//	{
//		if (IS_A(p,IsotropicPointSourceDescription,s))
//			os << "1 " << delim{""," ",""} << p->getOrigin() << endl;
//		else if (IS_A(p,VolumeSourceDescription,s))
//			os << "2 " << p->getIDt() << endl;
//		else
//			throw std::exception();
//
//		os << ' ' << s->getPower() << endl;
//	}
//
//	if(os.fail())
//		throw write_exception("TIMOSWriter::write(std::vector<Material>&) writing failed");
//}


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


//boost update hooman
	size_t nnz = 0;
	for(unsigned int i = 0; i < phi_v.size(); i ++)
		if(phi_v[i] != 0.0)
			nnz ++;

	 nnz = boost::size(phi_v | boost::adaptors::filtered([](double x) { return x != 0.0; }));

	os << "1 " << nnz << " 1" << endl;

	os.fill(' ');

	for(unsigned i=1; i<=mesh.getNt(); ++i)
		if (phi_v[i] > 0.0)
			os << setw(7) << delim{""," ",""} << mesh.getTetraPointIDs(i) <<
					fixed << setprecision(4) << setw(8) << mesh.getTetraVolume(i) << ' ' <<
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


//boost update hooman
	size_t nnz = 0;
	for(unsigned int i = 0; i < phi_s.size(); i ++)
		if(phi_s[i] != 0.0)
			nnz ++;

	//nnz = boost::size(phi_s | boost::adaptors::filtered([](double x) { return x != 0.0; }));

	os << "1 " << nnz << " 1" << endl;

	os.fill(' ');


	for(const auto& phi : phi_s | boost::adaptors::indexed(0U))
		if (phi.value() > 0.0)

			os << setw(7) << delim{""," ",""} << mesh.getFacePointIDs(phi.index()) <<
					fixed << setprecision(4) << setw(8) << mesh.getFaceArea((int)phi.index()) << ' ' <<
					scientific << setprecision(5) << setw(8) << phi.value() << endl;
}
