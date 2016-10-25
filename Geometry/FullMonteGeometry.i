%module FullMonteGeometryTCL

%include "std_string.i"
%include "std_vector.i"

%include "FullMonteGeometry_types.i"

%{

#include <FullMonteSW/Geometry/Sources/Abstract.hpp>
#include <FullMonteSW/Geometry/Sources/Ball.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>
#include <FullMonteSW/Geometry/Sources/Surface.hpp>
#include <FullMonteSW/Geometry/Sources/PointSource.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>
#include <FullMonteSW/OutputTypes/clonable.hpp>
#include <FullMonteSW/OutputTypes/visitable.hpp>

%}

%include "Sources/Abstract.hpp"
%include "Sources/Directed.hpp"
%include "Sources/PointSource.hpp"
%include "Sources/Volume.hpp"
%include "Sources/Ball.hpp"
%include "Sources/Composite.hpp"
%include "Sources/Surface.hpp"
%include "Sources/SurfaceTri.hpp"
%include "Sources/PencilBeam.hpp"

%{
#include <vector>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <sstream>
#include <FullMonteSW/Geometry/Filters/TetraMeshBaseFilter.hpp>
#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>
#include <FullMonteSW/Geometry/Filters/TetraFilterByRegion.hpp>
#include <FullMonteSW/Geometry/Filters/PlaneCrossingFilter.hpp>
%}

%nodefaultctor TetraMesh;
%nodefaultctor TetraMeshBase;
%include "TetraMeshBase.hpp"
%include "TetraMesh.hpp"

%include "Filters/FilterBase.hpp"
%template(FilterBaseI) FilterBase<int>;
%template(FilterBaseU) FilterBase<unsigned>;
%include "Filters/TetraMeshBaseFilter.hpp"
%include "Filters/TetraMeshFilterBase.hpp"
%include "Filters/TriFilterRegionBounds.hpp"
%include "Filters/TetraFilterByRegion.hpp"

%include "Filters/PlaneCrossingFilter.hpp"




