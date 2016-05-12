%module FullMonteTIMOSTCL

%include "std_string.i"

%include "std_vector.i"
%include "../../Geometry/FullMonteGeometry.i"
%include "../../Geometry/FullMonteGeometry_types.i"

%{
#include "TIMOS.hpp"
#include <FullMonte/Geometry/Sources/Base.hpp>
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/SimpleMaterial.hpp>
#include <FullMonte/Storage/TIMOS/TIMOS.hpp>
#include <FullMonte/Storage/TIMOS/TIMOSReader.hpp>
#include <FullMonte/Storage/TIMOS/TIMOSWriter.hpp>
#include <FullMonte/Storage/TIMOS/TIMOSAntlrParser.hpp>

%}

%include "TIMOS.hpp"
%include "TIMOSReader.hpp"
%include "TIMOSAntlrParser.hpp"
%include "TIMOSWriter.hpp"