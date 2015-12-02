#ifndef TIMOS_HPP_INCLUDED_
#define TIMOS_HPP_INCLUDED_


#include <FullMonte/Geometry/SimpleMaterial.hpp>
#include <FullMonte/Storage/CommonParser/ANTLRParser.hpp>


#include <FullMonte/Storage/Reader.hpp>

#ifndef SWIG




#include "TIMOSLexer.h"
#include "TIMOSParser.h"

#include <string>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>



#endif


class TIMOSReader : public Reader {
	std::string sourceFn_,optFn_,meshFn_,legendFn_;
public:
	TIMOSReader() {}
	TIMOSReader(std::string pfx) : sourceFn_(pfx+".source"),optFn_(pfx+".opt"),meshFn_(pfx+".mesh"),legendFn_(pfx+".legend"){}

	virtual ~TIMOSReader(){}

	void setMeshFileName(std::string fn){ meshFn_=fn; }
	void setOpticalFileName(std::string fn){ optFn_=fn; }
	void setSourceFileName(std::string fn){ sourceFn_=fn; }
	void setLegendFileName(std::string fn){ legendFn_=fn; }

	virtual TetraMesh						mesh() const;
	virtual std::vector<Material>			materials() const;
	virtual std::vector<SourceDescription*>	sources() const;

	virtual std::vector<SimpleMaterial>		materials_simple() const;

	virtual std::vector<LegendEntry> 		legend() const;

	virtual void clear(){}
};



/*
class TIMOSWriter : public Writer {

	std:: string sourceFn_, optFn_, meshFn_;

//	void doWriteSources() const;
//	void doWriteOptical() const;
//	void doWriteMesh() const;

	static void writeUserComments(std::ostream&,std::string);

public:
	TIMOSWriter(std::string pfx) : Writer(),sourceFn_(pfx+".source"),optFn_(pfx+".opt"),meshFn_(pfx+".mesh"){}

	virtual ~TIMOSWriter(){}

	virtual void write(const TetraMesh&) const{}
	virtual void write(const std::vector<SourceDescription*>&) const{}
	virtual void write(const std::vector<Material>&) const{}

	virtual void writeSurfFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
	virtual void writeVolFluence(std::string fn,const TetraMesh& mesh,const std::vector<double>& phi_s,std::string) const;
};

*/

#ifndef SWIG

namespace TIMOS {

struct Tetra {
	std::array<unsigned,4>	IDps=std::array<unsigned,4>{0,0,0,0};
	unsigned				region=0;

	Tetra(){}
	Tetra(std::array<unsigned,4> IDps_,unsigned region_) : IDps(IDps_),region(region_){}
};

struct Mesh {
	std::vector<std::array<double,3>> 	P;
	std::vector<Tetra> 					T;
};

struct Optical {
	struct Material {
		float mu_a=NAN;
		float mu_s=NAN;
		float g=NAN;
		float n=NAN;
	};

	std::vector<Material> mat;
	bool matched=false;
	bool by_region=false;
	float n_ext=NAN;
};

struct Source {
	unsigned type=0;
	unsigned w=0;

	enum Types { Point=1, Volume=2, PencilBeam=11, Face=12 };

	union {
		struct { std::array<double,3> pos; } point;
		struct { unsigned tetID; } vol;
		struct { std::array<double,3> pos,dir; unsigned tetID; } pencilbeam;
		struct { std::array<unsigned,3> IDps; } face;
	} details;

	friend std::ostream& operator<<(std::ostream& os,const Source& s);
};



Optical 					parse_optical(std::string fn);
std::vector<Source>			parse_sources(std::string fn);
Mesh						parse_mesh(std::string fn);
std::vector<LegendEntry> 	parse_legend(std::string fn);

};

#endif


#endif /* TIMOS_HPP_ */
