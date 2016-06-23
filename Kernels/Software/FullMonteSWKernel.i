%module FullMonteSWKernelTCL

%include "std_vector.i"
%include "std_string.i"

%include <FullMonteHW/Geometry/FullMonteGeometry_types.i>

%{
#include <FullMonteHW/Kernels/Kernel.hpp>
#include <FullMonteHW/Kernels/KernelObserver.hpp>
#include <FullMonteHW/Kernels/OStreamObserver.hpp>
#include <FullMonteHW/Kernels/MCKernelBase.hpp>
#include <FullMonteHW/Kernels/TetraKernelBase.hpp>
#include <FullMonteHW/Kernels/Software/TetraMCKernel.hpp>
#include <FullMonteHW/Kernels/Software/TetraSVKernel.hpp>
#include <FullMonteHW/Kernels/Software/TetraVolumeKernel.hpp>
#include <FullMonteHW/Kernels/Software/TetraSurfaceKernel.hpp>
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