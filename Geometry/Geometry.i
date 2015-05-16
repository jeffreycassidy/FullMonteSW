%module FullMonteGeometry_TCL

%include "std_string.i"
%include "std_vector.i"

%include "Geometry_types.i"


%{

#include <vector>
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/TriSurf.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>
#include <sstream>


%}

//%include "SourceDescription.hpp"
%include "TetraMeshBase.hpp"
%include "TetraMesh.hpp"