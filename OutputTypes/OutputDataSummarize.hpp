/*
 * OutputDataSummarize.hpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_OUTPUTDATASUMMARIZE_HPP_
#define OUTPUTTYPES_OUTPUTDATASUMMARIZE_HPP_

#include "OutputData.hpp"

class OutputDataSummarize : public OutputData::Visitor
{
public:
	OutputDataSummarize(std::ostream& os);

private:
	virtual void doVisit(MCConservationCountsOutput*) override;
	virtual void doVisit(MCEventCountsOutput*) override;
	virtual void doVisit(OutputData*) override;

	std::ostream& m_os;
};




#endif /* OUTPUTTYPES_OUTPUTDATASUMMARIZE_HPP_ */
