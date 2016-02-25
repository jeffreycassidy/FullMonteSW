%module FullMonteSWKernelTCL

%include "std_vector.i"
%include "std_string.i"

%include <FullMonte/Geometry/FullMonteGeometry_types.i>

%{
#include <FullMonte/Kernels/Kernel.hpp>
#include <FullMonte/Kernels/KernelObserver.hpp>
#include <FullMonte/Kernels/OStreamObserver.hpp>
#include <FullMonte/Kernels/MCKernelBase.hpp>
#include <FullMonte/Kernels/TetraKernelBase.hpp>
#include <FullMonte/Kernels/Software/TetraMCKernel.hpp>
#include <FullMonte/Kernels/Software/TetraSVKernel.hpp>
#include <FullMonte/Kernels/Software/TetraVolumeKernel.hpp>
#include <FullMonte/Kernels/Software/TetraSurfaceKernel.hpp>
%}

%include "../Kernel.hpp"

%include "MCKernelBase.hpp"
%include "ThreadedMCKernelBase.hpp"
%include "TetraKernelBase.hpp"
%include "TetraMCKernel.hpp"

%template (TetraMCKernelAVX) TetraMCKernel<RNG_SFMT_AVX>;


%include "TetraSVKernel.hpp"
%include "TetraVolumeKernel.hpp"
%include "TetraSurfaceKernel.hpp"