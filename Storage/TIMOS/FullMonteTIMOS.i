%module FullMonteTIMOS_TCL

%include "std_string.i"
%apply std::string { const std::string* }

%typemap(out) std::array<float,3> {
	stringstream ss;
	std::array<float,3> a=$1;
	for(unsigned i=0;i<3;++i)
		ss << a[i] << ' ';
	Tcl_AppendResult(interp,ss.str().c_str());
}

%{
	
#include "TIMOS.hpp"
#include <FullMonte/Geometry/SourceDescription.hpp>
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>

%}

%include "std_vector.i"
namespace std{
	%template(vectorlegend) vector<LegendEntry>;
	%template(sourcevector) vector<SourceDescription*>;
	%template(materialvector) vector<SimpleMaterial>;
};

struct LegendEntry {
	std::string label;
	std::array<float,3> colour;
};

struct SimpleMaterial {
	double mu_a,mu_s,g,n;
};


%include "TIMOS.hpp"