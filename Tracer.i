%module FullMonteTracer

%include "std_string.i"

%{

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>
#include <vector>
#include <string>

void TracerKernel(const TetraMesh& mesh,const std::vector<SimpleMaterial>& mats,const IsotropicPointSourceDescription& ips,unsigned long long Npkt,double units_per_cm,double E);

IsotropicPointSourceDescription newIPS(double x,double y,double z);

%}

%include "std_vector.i"
namespace std{
	%template(materialvector) vector<SimpleMaterial>;
};

void TracerKernel(const TetraMesh& mesh,const std::vector<SimpleMaterial>& mats,const IsotropicPointSourceDescription& ips,unsigned long long Npkt,double units_per_cm,double E);
IsotropicPointSourceDescription newIPS(double x,double y,double z);
