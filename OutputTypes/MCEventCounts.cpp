/*
 * MCEventCount.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#include "MCEventCounts.hpp"

template<>const std::string clonable<OutputData,MCEventCountsOutput,OutputData::Visitor>::s_typeString = "mc_events";

MCEventCounts& MCEventCounts::operator+=(const MCEventCounts& rhs)
{
    Nlaunch     += rhs.Nlaunch;
    Nabsorb     += rhs.Nabsorb;
    Nscatter    += rhs.Nscatter;
    Nbound      += rhs.Nbound;
    Ntir        += rhs.Ntir;
    Nfresnel    += rhs.Nfresnel;
    Nrefr       += rhs.Nrefr;
    Ninterface  += rhs.Ninterface;
    Nexit       += rhs.Nexit;
    Ndie        += rhs.Ndie;
    Nwin        += rhs.Nwin;
    Nabnormal	+= rhs.Nabnormal;
    Ntime       += rhs.Ntime;
    Nnohit      += rhs.Nnohit;
    return *this;
}

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT_GUID(MCEventCountsOutput,"MCEventCountsOutput")
