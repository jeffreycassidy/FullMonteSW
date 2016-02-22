/*
 * MCEventCount.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_MCEVENTCOUNTS_HPP_
#define OUTPUTTYPES_MCEVENTCOUNTS_HPP_

struct MCEventCounts {
    unsigned long long Nlaunch=0;
    unsigned long long Nabsorb=0;
    unsigned long long Nscatter=0;;
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

	void clear(){ *this = MCEventCounts(); }

	MCEventCounts& operator +=(const MCEventCounts&);
};

#include "OutputData.hpp"

class MCEventCountsOutput : public clonable<OutputData,MCEventCountsOutput,OutputData::Visitor>, public MCEventCounts
{
public:
	MCEventCountsOutput(const MCEventCounts& C=MCEventCounts()) : MCEventCounts(C){}
private:
	void acceptVisitor(OutputData::Visitor* v) override { v->doVisit(this); }
};


#endif /* OUTPUTTYPES_MCEVENTCOUNTS_HPP_ */
