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

	void operator()(OutputData* d){ return visit(d); }

private:
	virtual void doVisit(MCConservationCountsOutput*) override;
	virtual void doVisit(MCEventCountsOutput*) override;
	virtual void doVisit(OutputData*) override;
	virtual void doVisit(VolumeAbsorbedEnergyMap*) override;
	virtual void doVisit(SurfaceExitEnergyMap*) override;
	virtual void doVisit(SurfaceFluenceMap*) override;
	virtual void doVisit(VolumeFluenceMap*) override;

	std::ostream& m_os;
};




#endif /* OUTPUTTYPES_OUTPUTDATASUMMARIZE_HPP_ */
