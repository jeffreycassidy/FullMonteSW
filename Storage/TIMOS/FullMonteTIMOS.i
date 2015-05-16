%module FullMonteTIMOS_TCL

%include "std_string.i"
%apply std::string { const std::string* }

%include "../../Geometry/Geometry_types.i"

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