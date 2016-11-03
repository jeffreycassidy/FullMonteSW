/*
 * MCEventCount.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_MCEVENTCOUNTS_HPP_
#define OUTPUTTYPES_MCEVENTCOUNTS_HPP_

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

class OutputData;

struct MCEventCounts
{
    unsigned long long Nlaunch=0;
    unsigned long long Nabsorb=0;
    unsigned long long Nscatter=0;
    unsigned long long Nbound=0;
    unsigned long long Ntir=0;
    unsigned long long Nfresnel=0;
	unsigned long long Nrefr =0;
	unsigned long long Ninterface=0;
	unsigned long long Nexit=0;
	unsigned long long Ndie=0;
	unsigned long long Nwin=0;
	unsigned long long Nabnormal=0;
	unsigned long long Ntime=0;
	unsigned long long Nnohit=0;

	MCEventCounts& operator +=(const MCEventCounts&);

	void clear(){ *this = MCEventCounts(); }
};

OutputData* createOutputData(const MCEventCounts& C);

#include "OutputData.hpp"


class MCEventCountsOutput :
		public OutputData,
		public MCEventCounts
{
public:
	MCEventCountsOutput(const MCEventCounts& C=MCEventCounts()) : MCEventCounts(C){}

	CLONE_METHOD(OutputData,MCEventCountsOutput)
	ACCEPT_VISITOR_METHOD(OutputData,MCEventCountsOutput)

	virtual const char* typeString() const override { return "MC Event Counts"; }

private:

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OutputData)
			& BOOST_SERIALIZATION_NVP(Nlaunch)
			& BOOST_SERIALIZATION_NVP(Nabsorb)
			& BOOST_SERIALIZATION_NVP(Nscatter)
			& BOOST_SERIALIZATION_NVP(Nbound)
			& BOOST_SERIALIZATION_NVP(Ntir)
			& BOOST_SERIALIZATION_NVP(Nfresnel)
			& BOOST_SERIALIZATION_NVP(Nrefr)
			& BOOST_SERIALIZATION_NVP(Ninterface)
			& BOOST_SERIALIZATION_NVP(Nexit)
			& BOOST_SERIALIZATION_NVP(Ndie)
			& BOOST_SERIALIZATION_NVP(Nwin)
			& BOOST_SERIALIZATION_NVP(Nabnormal)
			& BOOST_SERIALIZATION_NVP(Ntime)
			& BOOST_SERIALIZATION_NVP(Nnohit);
		}

	friend class boost::serialization::access;
};


#endif /* OUTPUTTYPES_MCEVENTCOUNTS_HPP_ */
