%module FullMonteVolume_TCL

%include "std_string.i"


%{

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>
#include <vector>
#include <string>

std::vector<double> VolumeKernel(const TetraMesh& mesh,const std::vector<SimpleMaterial>& mats,const IsotropicPointSourceDescription& ips,unsigned long long Npkt,double units_per_cm,double E);

%}


%include "std_vector.i"
namespace std{
	%template(materialvector) vector<SimpleMaterial>;
};

std::vector<double> VolumeKernel(const TetraMesh& mesh,const std::vector<SimpleMaterial>& mats,const IsotropicPointSourceDescription& ips,unsigned long long Npkt,double units_per_cm,double E);