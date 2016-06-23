%module FullMonteKernelsTCL

%include "std_vector.i"
%include "std_string.i"

%include "../Geometry/FullMonteGeometry_types.i"

%{
#include "Kernel.hpp"
#include "KernelObserver.hpp"
#include "OStreamObserver.hpp"
#include "MCKernelBase.hpp"
#include "TetraKernelBase.hpp"
#include <FullMonteSW/Geometry/TetraMeshBase.hpp>
#include <FullMonteSW/Geometry/TetraMesh.hpp>
%}

%include "Kernel.hpp"
%include "KernelObserver.hpp"
%include "OStreamObserver.hpp"
%include "MCKernelBase.hpp"
%include "TetraKernelBase.hpp"
