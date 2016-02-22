%module FullMonteKernelsTCL

%include "std_vector.i"
%include "std_string.i"

%template(DoubleVector) std::vector<double>;

%{
#include "Kernel.hpp"
#include "KernelObserver.hpp"
#include "OStreamObserver.hpp"
#include "MCKernelBase.hpp"
#include "TetraKernelBase.hpp"
%}

%template(materialvector) std::vector<SimpleMaterial>;

%include "Kernel.hpp"
%include "KernelObserver.hpp"
%include "OStreamObserver.hpp"
%include "MCKernelBase.hpp"
%include "TetraKernelBase.hpp"
%include "../Geometry/FullMonteGeometry_types.i"
