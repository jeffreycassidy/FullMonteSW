%module FullMonteVTKFileTCL

%include "std_string.i"

%include "VTKLegacyReader.hpp"
%include "VTKLegacyWriter.hpp"

%{
	#include <FullMonte/Storage/VTK/VTKLegacyReader.hpp>
	#include <FullMonte/Storage/VTK/VTKLegacyWriter.hpp>
%}

class TetraMeshBase {};

class TetraMesh : public TetraMeshBase {};