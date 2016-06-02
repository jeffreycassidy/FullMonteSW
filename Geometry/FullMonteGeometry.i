%module FullMonteGeometryTCL

%include "std_string.i"
%include "std_vector.i"

%include "FullMonteGeometry_types.i"

%{

#include <FullMonteSW/Geometry/Sources/Base.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>
#include <FullMonteSW/Geometry/Sources/Surface.hpp>
#include <FullMonteSW/Geometry/Sources/PointSource.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>

#include <FullMonteSW/Geometry/Sources/Visitor.hpp>

using namespace Source;
using namespace Source::detail;

namespace Source { namespace detail { template<class Base,class Derived>class cloner; }; };

%}

%include "Sources/Base.hpp"
%include "Sources/PointSource.hpp"

%include "Sources/Volume.hpp"

%include "Sources/Composite.hpp"
%include "Sources/Surface.hpp"
%include "Sources/SurfaceTri.hpp"

// for some reason, the line of code below disables all constructors (??)
//%template(PencilBeamCloner) Source::detail::cloner<Source::PointSource,Source::PencilBeam>;
%include "Sources/PencilBeam.hpp"




%{
#include <vector>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <sstream>
%}
%include "TetraMeshBase.hpp"
%include "TetraMesh.hpp"


