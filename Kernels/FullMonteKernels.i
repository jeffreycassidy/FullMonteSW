%module FullMonteKernelsTCL

%include "std_vector.i"
%include "std_string.i"

%template(DoubleVector) std::vector<double>;

%{
#include "Kernel.hpp"
#include "KernelObserver.hpp"
#include "Software/OStreamObserver.hpp"
%}

%template(materialvector) std::vector<SimpleMaterial>;
%template(sourcedescriptionvector) std::vector<SourceDescription*>;


%include "Kernel.hpp"
%include "KernelObserver.hpp"
%include "Software/OStreamObserver.hpp"
%include "../Geometry/FullMonteGeometry_types.i"
