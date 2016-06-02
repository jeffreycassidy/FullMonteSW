%module FullMonteVTKFileTCL

%include "std_string.i"

%include "VTKLegacyReader.hpp"
%include "VTKLegacyWriter.hpp"

%{
	#include <FullMonteSW/Storage/VTK/VTKLegacyReader.hpp>
	#include <FullMonteSW/Storage/VTK/VTKLegacyWriter.hpp>
%}