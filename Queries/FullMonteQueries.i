%module FullMonteQueriesTCL

//#include <FullMonteSW/Queries/FluenceConverter.hpp>
//#include <FullMonteSW/Queries/CrossingFluence.hpp>
//#include <FullMonteSW/Queries/DoseSurfaceHistogramGenerator.hpp>
//#include <FullMonteSW/Queries/DoseVolumeHistogramGenerator.hpp>
//template class EmpiricalCDF<float,float,std::less<float>>;

//%include <FullMonteSW/Queries/CrossingFluence.hpp>
//%include <FullMonteSW/Queries/DoseSurfaceHistogramGenerator.hpp>
//%include <FullMonteSW/Queries/FluenceConverter.hpp>
//%include <FullMonteSW/Queries/DoseVolumeHistogramGenerator.hpp>
//%include <FullMonteSW/Queries/EmpiricalCDF.hpp>

%{
#include "DirectionalSurface.hpp"
#include "EnergyToFluence.hpp"
#include "DoseHistogramGenerator.hpp"
#include "DoseHistogram.hpp"
%}

%include "DirectionalSurface.hpp"
%include "EnergyToFluence.hpp"
%include "DoseHistogramGenerator.hpp"
%include "DoseHistogram.hpp"

//template<typename Value,typename Weight,typename Comp> class EmpiricalCDF;
//%template(DoseSurfaceHistogramF) EmpiricalCDF<float,float,std::less<float>>;
//%template(DoseVolumeHistogramF) EmpiricalCDF<float,float,std::less<float>>;