#ifndef TIMOS_HPP_INCLUDED_
#define TIMOS_HPP_INCLUDED_


#include <FullMonte/Geometry/SimpleMaterial.hpp>

#include <FullMonte/Geometry/Sources/Base.hpp>

#include <string>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>

namespace Source { class Base; };

class TIMOS
{

public:
	TIMOS(){}
	TIMOS(std::string pfx) : sourceFn_(pfx+".source"),optFn_(pfx+".opt"),meshFn_(pfx+".mesh"),legendFn_(pfx+".legend")
	{
	}

	virtual ~TIMOS(){}

	void setMeshFileName(std::string fn){ meshFn_=fn; }
	void setOpticalFileName(std::string fn){ optFn_=fn; }
	void setSourceFileName(std::string fn){ sourceFn_=fn; }
	void setLegendFileName(std::string fn){ legendFn_=fn; }

	struct LegendEntry {
		std::string 		label;
		std::array<float,3> colour;
	};

protected:
	struct SourceDef {
		unsigned type=0;
		unsigned w=0;

		enum Types { Point=1, Volume=2, PencilBeam=11, Face=12 };

		union {
			struct { std::array<float,3> pos; } 						point;
			struct { unsigned tetID; } 									vol;
			struct { std::array<float,3> pos,dir; unsigned tetID; } 	pencilbeam;
			struct { std::array<unsigned,3> IDps; } 					face;
		} details;

		friend std::ostream& operator<<(std::ostream& os,const SourceDef& s);
	};


	struct Tetra {
		std::array<unsigned,4>	IDps=std::array<unsigned,4>{{0,0,0,0}};
		unsigned				region=0;

		Tetra(){}
		Tetra(std::array<unsigned,4> IDps_,unsigned region_) : IDps(IDps_),region(region_){}
	};

	struct Mesh {
		std::vector<std::array<float,3>> 	P;
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



	static Source::Base* 	convertToSource(SourceDef);
	static SourceDef 		convertFromSource(const Source::Base*);

	std::string sourceFn_,optFn_,meshFn_,legendFn_;

	friend std::ostream& operator<<(std::ostream& os,const TIMOS::SourceDef& s);
};

inline void ping(float w,std::array<float,3> a,std::array<float,3> b)
{
	std::cout << " Hello! w=" << w << " Array is " << a[0] << ' ' << a[1] << ' ' << a[2] << " -- " << b[0] << ' ' << b[1] << ' ' << b[2] << std::endl;
}

#endif /* TIMOS_HPP_ */
