%module FullMonteDataTCL

%{
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "FluenceConverter.hpp"
#include "CrossingFluence.hpp"
//#include "DoseSurfaceHistogramGenerator.hpp"
%}



%include "std_string.i"
%include "CrossingFluence.hpp"
//%include "DoseSurfaceHistogramGenerator.hpp"
%include "FluenceConverter.hpp"
//%include "EmpiricalCDF.hpp"

//template<typename Value,typename Weight,typename Comp> class EmpiricalCDF;
//%template(DoseSurfaceHistogramF) EmpiricalCDF<float,float,std::less<float>>;
