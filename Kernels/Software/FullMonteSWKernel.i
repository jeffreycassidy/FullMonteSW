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
#include <FullMonteSW/Kernels/Software/TetraTraceKernel.hpp>
%}

%include "../Kernel.hpp"

%include "MCKernelBase.hpp"
%include "ThreadedMCKernelBase.hpp"
%include "TetraKernelBase.hpp"
%include "TetraMCKernel.hpp"


%template (TetraMCKernelAVX_V) TetraMCKernel<RNG_SFMT_AVX,TetraVolumeScorer>;
%template (TetraMCKernelAVX_S) TetraMCKernel<RNG_SFMT_AVX,TetraSurfaceScorer>;
%template (TetraMCKernelAVX_SV) TetraMCKernel<RNG_SFMT_AVX,SVScorer>;
%template (TetraMCKernelAVX_T) TetraMCKernel<RNG_SFMT_AVX,TraceScorer>;

%include "TetraTraceKernel.hpp"
%include "TetraSVKernel.hpp"
%include "TetraVolumeKernel.hpp"
%include "TetraSurfaceKernel.hpp"