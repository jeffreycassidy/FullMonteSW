%module FullMonteGeometryTCL

%include "std_string.i"
%include "std_vector.i"

%include "FullMonteGeometry_types.i"

%{

#include <FullMonteSW/Geometry/Sources/Base.hpp>
#include <FullMonteSW/Geometry/Sources/Ball.hpp>
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
%include "Sources/Directed.hpp"

%template (SourceBase_Point)  		Source::detail::cloner<Source::Base,Source::PointSource>;
%include "Sources/PointSource.hpp"

%template (SourceBase_Volume)		Source::detail::cloner<Source::Base,Source::Volume>;
%include "Sources/Volume.hpp"

%template (SourceBase_Ball)   		Source::detail::cloner<Source::PointSource,Source::Ball>;
%include "Sources/Ball.hpp"

%template (SourceBase_Composite)	Source::detail::cloner<Source::Base,Source::Composite>;
%include "Sources/Composite.hpp"

%template (SourceBase_Surface)	Source::detail::cloner<Source::Base,Source::Surface>;
%include "Sources/Surface.hpp"

%template (SourceBase_SurfaceTri)   Source::detail::cloner<Source::Surface,Source::SurfaceTri>;
%include "Sources/SurfaceTri.hpp"

%template (SourceBase_Pencil) 		Source::detail::cloner<Source::PointSource,Source::PencilBeam>;
%include "Sources/PencilBeam.hpp"



%{
#include <vector>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <sstream>
#include <FullMonteSW/Geometry/Filters/TetraMeshBaseFilter.hpp>
#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>
%}

%nodefaultctor TetraMesh;
%nodefaultctor TetraMeshBase;
%include "TetraMeshBase.hpp"
%include "TetraMesh.hpp"

%include "Filters/FilterBase.hpp"
%template(FilterBaseI) FilterBase<int>;
%template(FilterBaseU) FilterBase<unsigned>;
%include "Filters/TetraMeshBaseFilter.hpp"
%include "Filters/TetraMeshFilter.hpp"
%include "Filters/TriFilterRegionBounds.hpp"




