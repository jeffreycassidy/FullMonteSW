/*
 * OutputDataSummarize.cpp
 *
 *  Created on: Feb 5, 2016
 *      Author: jcassidy
 */

#include "OutputDataSummarize.hpp"
#include "MCConservationCounts.hpp"
#include "MCEventCounts.hpp"
#include "FluenceMapBase.hpp"

#include <iostream>

using namespace std;

OutputDataSummarize::OutputDataSummarize(std::ostream& os) :
	m_os(os)
{
}


/** Outputs a text summary of conservation statistics.
 * Difference output should be minimal.
 */

void OutputDataSummarize::doVisit(MCConservationCountsOutput* cc)
{
    double w_dispose = cc->w_absorb+cc->w_exit+cc->w_time;
    double w_diff = w_dispose-cc->w_launch;

    m_os << "Energy conservation" << endl;
    m_os << "  Launched: " << cc->w_launch << endl;
    m_os << "  Disposed: " << w_dispose << endl;
    m_os << "    Absorbed   " << cc->w_absorb << endl;
    m_os << "    Exited     " << cc->w_exit << endl;
    m_os << "    Time gated " << cc->w_time << endl;
    m_os << "  Difference: " << w_diff << " (" << 100.0*w_diff/cc->w_launch << "%)" << endl;
    m_os << endl;

    m_os << "Roulette difference " << cc->w_roulette-cc->w_die << endl;
    m_os << "  Died " << cc->w_die << endl;
    m_os << "  Added " << cc->w_roulette << endl << endl;

    m_os << "Abnormal termination (total " << cc->w_abnormal+cc->w_nohit << ')' << endl;
    m_os << "  No intersection: " << cc->w_nohit << endl;
    m_os << "  Excessive steps: " << cc->w_abnormal << endl;
}

void OutputDataSummarize::doVisit(OutputData* d)
{
	m_os << "OutputDataSummarize visitor not defined for OutputData of type '" << d->typeString() << "'" << endl;
}

void OutputDataSummarize::doVisit(VolumeAbsorbedEnergyMap* em)
{
	m_os << "Volume absorbed energy map of dimension " << (*em)->dim() << " with total " << (*em)->sum() << " (" << (*em)->nnz() << " nonzeros, " << em->totalEmitted() << " emitted)" << endl;
}

void OutputDataSummarize::doVisit(SurfaceExitEnergyMap* sm)
{
	m_os << "Surface exit energy map of dimension " << (*sm)->dim() << " with total " << (*sm)->sum() << " (" << (*sm)->nnz() << " nonzeros, " << sm->totalEmitted() << " emitted)" << endl;
}


void OutputDataSummarize::doVisit(MCEventCountsOutput* ec)
{
    m_os << "Launched: " << ec->Nlaunch << endl;

    m_os << "Boundary (same):      " << ec->Nbound << endl;
    m_os << "Boundary (different): " << ec->Ninterface << endl;
    m_os << "  TIR:     " << ec->Ntir << endl;
    m_os << "  Fresnel: " << ec->Nfresnel << endl;
    m_os << "  Refract: " << ec->Nrefr << endl;
    m_os << "  Balance (bound - [TIR + fresnel + refract]): " << ec->Ninterface-ec->Ntir-ec->Nfresnel-ec->Nrefr << endl;

    m_os << "Absorption: " << ec->Nabsorb << endl;
    m_os << "Scatter:    " << ec->Nscatter << endl;

    m_os << "Roulette results" << endl;
    m_os << "  Win:  " << ec->Nwin << endl;
    m_os << "  Lose: " << ec->Ndie << endl;

    m_os << "End results" << endl;
    m_os << "Died:       " << ec->Ndie << endl;
    m_os << "Exited:     " << ec->Nexit << endl;
    m_os << "Abnormal:   " << ec->Nabnormal << endl;
    m_os << "Time gated: " << ec->Ntime << endl;
    m_os << "No hit:     " << ec->Nnohit << endl;
    m_os << "Balance ([launch] - [die + exit]): " << ec->Nlaunch-ec->Ndie-ec->Nexit-ec->Ntime-ec->Nabnormal-ec->Nnohit << endl;
}

void OutputDataSummarize::doVisit(SurfaceFluenceMap* sf)
{

	m_os << "OutputDataSummarize visitor not defined for OutputData of type '" << sf->typeString() << "'" << endl;
}

void OutputDataSummarize::doVisit(VolumeFluenceMap* vf)
{

	m_os << "OutputDataSummarize visitor not defined for OutputData of type '" << vf->typeString() << "'" << endl;
}
