/*
 * BinFile.cpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#include "BinFile.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/utility.hpp>

#include <vector>
#include <array>

#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>

using namespace std;
namespace boost {
namespace serialization {

template<class Archive>void serialize(Archive& ar,TetraByPointID& T_p,const unsigned){ ar & T_p[0] & T_p[1] & T_p[2] & T_p[3]; }

}} // namespace boost

TetraMesh BinFileReader::mesh() const
{
	ifstream is(meshFn_.c_str());
	boost::archive::binary_iarchive ia(is);

	vector<array<float,3>> 	P;
	vector<TetraByPointID>	T_p;
	vector<unsigned char>	T_m;

	ia & P & T_p & T_m;

	vector<Point<3,double>> Pd(P.size());
	vector<unsigned>		T_mu(T_p.size());

	// Conversions are OK because they are both widening
	boost::copy(P | boost::adaptors::transformed(std::function<Point<3,double>(array<float,3>)>([](array<float,3> a){ return Point<3,double>{ a[0],a[1],a[2] }; })),
			Pd.begin());
	boost::copy(T_m, T_mu.begin());

	return TetraMesh(Pd,T_p,T_mu);
}

vector<Material> BinFileReader::materials() const { cerr << "BinFileReader::materials() not supported"; throw std::logic_error("BinFileReader::materials() not supported"); }
vector<SourceDescription*> BinFileReader::sources() const { cerr << "BinFileReader::sources() not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }
vector<SimpleMaterial> BinFileReader::materials_simple() const { cerr << "BinFileReader::materials_simple() not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }
vector<LegendEntry> BinFileReader::legend() const { cerr << "BinFileReader::legend() not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }

void BinFileWriter::write(const TetraMesh& M) const
{
	ofstream os(meshFn_.c_str());
	boost::archive::binary_oarchive oa(os);

	vector<array<float,3>> 	Pf(M.getNp()+1);
	vector<unsigned char> 	T_mc(M.getNt()+1);

	// narrowing double->float conversion
	boost::copy(M.getPoints() | boost::adaptors::transformed(std::function<array<float,3>(Point<3,double>)>([](Point<3,double> P){ return array<float,3>{ (float)P[0],(float)P[1],(float)P[2]}; })),
			Pf.begin());

	// narrowing unsigned -> unsigned char (byte) conversion with check
	if (boost::algorithm::any_of(M.getMaterials(), std::function<int(unsigned)>([](unsigned i){ return i > 255; })))
	{
		cerr << "ERROR: Material index > 255 in BinFileWriter (invalid; uses unsigned char type)" << endl;
		throw std::out_of_range("ERROR: Material index > 255 in BinFileWriter (invalid; uses unsigned char type)");
	}
	boost::copy(M.getMaterials(), T_mc.begin());

	oa & Pf & M.getTetrasByPointID() & T_mc;
};

void BinFileWriter::write(const std::vector<SourceDescription*>&) const{ cerr << "BinFileWriter::write(vector<SourceDescription>) not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }
void BinFileWriter::write(const std::vector<Material>&) const{ cerr << "BinFileWriter::materials(vector<SourceDescription>) not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }

void BinFileWriter::writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const{ cerr << "BinFileWriter::materials(vector<SurfaceFluence>) not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }
void BinFileWriter::writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const{ cerr << "BinFileWriter::materials(vector<VolFluence>) not supported"; throw std::out_of_range("BinFileReader::materials() not supported"); }

