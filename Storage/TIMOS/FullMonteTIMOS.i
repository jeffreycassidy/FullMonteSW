%module FullMonteTIMOSTCL

%include "std_string.i"

%include "std_vector.i"
//%include "../../Geometry/FullMonteGeometry.i"
%include "../../Geometry/FullMonteGeometry_types.i"

%{
#include "TIMOS.hpp"
#include <FullMonteSW/Geometry/Sources/Abstract.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/SimpleMaterial.hpp>
#include <FullMonteSW/Storage/TIMOS/TIMOS.hpp>
#include <FullMonteSW/Storage/TIMOS/TIMOSReader.hpp>
#include <FullMonteSW/Storage/TIMOS/TIMOSWriter.hpp>
#include <FullMonteSW/Storage/TIMOS/TIMOSAntlrParser.hpp>

%}

%include "TIMOS.hpp"
%include "TIMOSReader.hpp"
%include "TIMOSAntlrParser.hpp"
%include "TIMOSWriter.hpp"
