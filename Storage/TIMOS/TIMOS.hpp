/*
 * TIMOS.hpp
 *
 *  Created on: Mar 5, 2015
 *      Author: jcassidy
 */

#ifndef TIMOS_HPP_
#define TIMOS_HPP_

#include <array>
#include <vector>
#include <cmath>

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
		struct { } face;
	} details;

	friend std::ostream& operator<<(std::ostream& os,const Source& s);
};


Optical 			parse_optical(std::string fn);
std::vector<Source>	parse_sources(std::string fn);
Mesh				parse_mesh(std::string fn);

};


#endif /* TIMOS_HPP_ */
