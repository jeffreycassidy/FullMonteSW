/*
 * OutputData.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#include "OutputData.hpp"

template<>const std::string clonable_base<OutputData>::s_typeString="<clonable_base:abstract-base-type>";

class VolumeAbsorbedEnergyMap;
class SurfaceExitEnergyMap;
class SurfaceFluenceMap;
class VolumeFluenceMap;

template<>const std::string clonable<OutputData,VolumeAbsorbedEnergyMap,OutputData::Visitor>::s_typeString="volume_energy";
template<>const std::string clonable<OutputData,SurfaceExitEnergyMap,OutputData::Visitor>::s_typeString="surface_energy";
template<>const std::string clonable<OutputData,SurfaceFluenceMap,OutputData::Visitor>::s_typeString="surface_fluence";
template<>const std::string clonable<OutputData,VolumeFluenceMap,OutputData::Visitor>::s_typeString="volume_fluence";
