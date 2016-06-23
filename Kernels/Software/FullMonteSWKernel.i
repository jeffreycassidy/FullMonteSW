%module FullMonteSWKernelTCL

%include "std_vector.i"
%include "std_string.i"

%include <FullMonteSW/Geometry/FullMonteGeometry_types.i>

%{
#include <FullMonteSW/Kernels/Kernel.hpp>
#include <FullMonteSW/Kernels/KernelObserver.hpp>
#include <FullMonteSW/Kernels/OStreamObserver.hpp>
#include <FullMonteSW/Kernels/MCKernelBase.hpp>
#include <FullMonteSW/Kernels/TetraKernelBase.hpp>
#include <FullMonteSW/Kernels/Software/TetraMCKernel.hpp>
#include <FullMonteSW/Kernels/Software/TetraSVKernel.hpp>
#include <FullMonteSW/Kernels/Software/TetraVolumeKernel.hpp>
#include <FullMonteSW/Kernels/Software/TetraSurfaceKernel.hpp>
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