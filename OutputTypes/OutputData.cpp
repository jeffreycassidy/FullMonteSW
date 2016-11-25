/*
 * OutputData.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#include "OutputData.hpp"

void OutputData::acceptVisitor(Visitor* v)
{
	v->doVisit(this);
}

//template<>const std::string clonable_base<OutputData>::s_typeString="<clonable_base:abstract-base-type>";
//
//template<>const std::string clonable<OutputData,VolumeAbsorbedEnergyMap,OutputData::Visitor>::s_typeString="volume_energy";
//template<>const std::string clonable<OutputData,SurfaceExitEnergyMap,OutputData::Visitor>::s_typeString="surface_energy";
//template<>const std::string clonable<OutputData,SurfaceFluenceMap,OutputData::Visitor>::s_typeString="surface_fluence";
//template<>const std::string clonable<OutputData,VolumeFluenceMap,OutputData::Visitor>::s_typeString="volume_fluence";
//template<>const std::string clonable<OutputData,FluenceLineQuery,OutputData::Visitor>::s_typeString="line_fluence";
//template<>const std::string clonable<OutputData,InternalSurfaceFluenceMap,OutputData::Visitor>::s_typeString="internal_surface_fluence";
//template<>const std::string clonable<OutputData,InternalSurfaceEnergyMap,OutputData::Visitor>::s_typeString="internal_surface_energy";
//template<>const std::string clonable<OutputData,VolumeAbsorbedEnergyDensityMap,OutputData::Visitor>::s_typeString="volume_energy_density";
