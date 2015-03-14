%module FullMonteBLI_TCL

%include "std_string.i"

%{

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/Material.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>
#include <vector>
#include <string>

void write_fluence(const std::string fn,const TetraMesh& mesh,const std::vector<double>& E,const std::string comm="");
std::vector<double> BLIKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const std::vector<SourceDescription*>& src,unsigned long long Npkt);
std::vector<double> BLIKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const BallSourceDescription& bsd,unsigned long long Npkt);

%}


void write_fluence(const std::string fn,const TetraMesh& mesh,const std::vector<double>& E,const std::string comm="");
std::vector<double> BLIKernel(const TetraMesh& mesh,const std::vector<Material>& mats,const BallSourceDescription& bsd,unsigned long long Npkt);
