/*
 * MCConservationCounts.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_MCCONSERVATIONCOUNTS_HPP_
#define OUTPUTTYPES_MCCONSERVATIONCOUNTS_HPP_

#include <boost/serialization/nvp.hpp>

struct MCConservationCounts
{
	MCConservationCounts(){}
	MCConservationCounts(const MCConservationCounts&) = default;

    double w_launch=0.0;			///< Amount of energy launched (generally 1.0 * Npackets)
    double w_absorb=0.0;			///< Amount of energy absorbed
    double w_die=0.0;				///< Amount of energy terminated in roulette
    double w_exit=0.0;				///< Amount of energy exiting
    double w_roulette=0.0;			///< Amount of energy added by winning roulette
    double w_abnormal=0.0;			///< Amount of energy terminated due to abnormal circumstances
    double w_time=0.0;				///< Amount of energy terminated due to time gate expiry
    double w_nohit=0.0;				///< Amount of energy terminated for failure to find an intersecting face

    /// Add another ConservationCounts
    MCConservationCounts& operator+=(const MCConservationCounts&);

};

#include "OutputData.hpp"

//template<class Wrapped>class OutputDataWrapper : public clonable<OutputData,Wrapped>, public Wrapped
//{
//public:
//
//private:
//	void acceptVisitor(OutputData::Visitor* v){ v->doVisit(this); }
//};

//typedef OutputDataWrapper<MCConservationCounts> MCConservationCountsOutput;


class MCConservationCountsOutput : public clonable<OutputData,MCConservationCountsOutput,OutputData::Visitor>, public MCConservationCounts
{
public:
	MCConservationCountsOutput() : MCConservationCounts(){}
	MCConservationCountsOutput(const MCConservationCounts& C) : MCConservationCounts(C){}

private:
	void acceptVisitor(OutputData::Visitor* v){ v->doVisit(this); }

private:
    template<class Archive>void serialize(Archive& ar,const unsigned ver)
    {
    	ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OutputData)
    		& boost::serialization::make_nvp("launched",w_launch)
    		& boost::serialization::make_nvp("absorbed",w_absorb)
    		& boost::serialization::make_nvp("died",w_die)
    		& boost::serialization::make_nvp("exited",w_exit)
    		& boost::serialization::make_nvp("won",w_roulette)
    		& boost::serialization::make_nvp("abnormal",w_abnormal)
    		& boost::serialization::make_nvp("time",w_time)
    		& boost::serialization::make_nvp("nohit",w_nohit);
    }
    friend class boost::serialization::access;
};


#endif /* OUTPUTTYPES_MCCONSERVATIONCOUNTS_HPP_ */
