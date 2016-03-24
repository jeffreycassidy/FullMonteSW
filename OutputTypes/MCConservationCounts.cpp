#include "MCConservationCounts.hpp"

template<>const std::string clonable<OutputData,MCConservationCountsOutput,OutputData::Visitor>::s_typeString = "conservation_counts";

/// Add another ConservationCounts object to the current
MCConservationCounts& MCConservationCounts::operator+=(const MCConservationCounts& cc)
{
	w_launch 	+= cc.w_launch;
	w_absorb 	+= cc.w_absorb;
	w_die    	+= cc.w_die;
	w_exit   	+= cc.w_exit;
	w_roulette 	+= cc.w_roulette;
	w_abnormal 	+= cc.w_abnormal;
	w_nohit    	+= cc.w_nohit;
	w_time     	+= cc.w_time;
	return *this;
}


