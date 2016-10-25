/*
 * vtkFullMontePacketPositionTraceToPolyData.cpp
 *
 *  Created on: Mar 6, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/OutputTypes/PacketPositionTrace.hpp>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <vtkObjectFactory.h>
#include "vtkFullMontePacketPositionTraceSetToPolyData.h"

vtkStandardNewMacro(vtkFullMontePacketPositionTraceSetToPolyData);

vtkFullMontePacketPositionTraceSetToPolyData::~vtkFullMontePacketPositionTraceSetToPolyData()
{
	m_vtkPD->Delete();
	m_vtkWeight->Delete();
	m_vtkTime->Delete();
	m_vtkLength->Delete();
	m_vtkStepCount->Delete();
}

vtkFullMontePacketPositionTraceSetToPolyData::vtkFullMontePacketPositionTraceSetToPolyData()
{
	m_vtkPD = vtkPolyData::New();

	if(m_includeWeight)
	{
		m_vtkWeight = vtkFloatArray::New();
		m_vtkWeight->SetName("Packet Weight");
		m_vtkPD->GetPointData()->AddArray(m_vtkWeight);
	}

	if(m_includeTime)
	{
		m_vtkTime = vtkFloatArray::New();
		m_vtkTime->SetName("Time of flight");
		m_vtkPD->GetPointData()->AddArray(m_vtkTime);
	}

	if(m_includeLength)
	{
		m_vtkLength = vtkFloatArray::New();
		m_vtkLength->SetName("Path length");
		m_vtkPD->GetPointData()->AddArray(m_vtkLength);
	}

	if(m_includeSteps)
	{
		m_vtkStepCount = vtkUnsignedShortArray::New();
		m_vtkStepCount->SetName("Step count");
		m_vtkPD->GetPointData()->AddArray(m_vtkStepCount);
	}
}

void vtkFullMontePacketPositionTraceSetToPolyData::source(const PacketPositionTrace* q)
{
	m_traces=q;
}

void vtkFullMontePacketPositionTraceSetToPolyData::source(const char* swigPtrString)
{

}

const PacketPositionTrace* vtkFullMontePacketPositionTraceSetToPolyData::source() const
{
	return m_traces;
}


void vtkFullMontePacketPositionTraceSetToPolyData::update()
{
	vtkIdType Np = m_traces->nPoints();	// number of points (total over all traces)
	vtkIdType Nt = m_traces->nTraces();	// number of traces

	// get/clear points
	vtkPoints* P = m_vtkPD->GetPoints();

	P->SetNumberOfPoints(Np);

	m_vtkWeight->SetNumberOfTuples(m_includeWeight ? Np : 0);
	m_vtkTime->SetNumberOfTuples(m_includeTime ? Np : 0);
	m_vtkLength->SetNumberOfTuples(m_includeLength ? Np : 0);
	m_vtkStepCount->SetNumberOfTuples(m_includeSteps ? Np : 0);

	// cell connectivity (0..N-1)
	vtkIdTypeArray* IDps = vtkIdTypeArray::New();
		IDps->SetNumberOfTuples(Np+Nt);


	vtkIdType IDp=0;

	vtkIdType i=0;			// Cell array counter (increments 1x/trace + 1x/point)

	for(const auto& trace : *m_traces)
	{

		unsigned Nstep=0;
		IDps->SetValue(i,trace.count());
		++i;

		for(const auto seg : m_trace->steps())
		{
			P->SetPoint(IDp,step.pos.data());
			IDps->SetValue(i,IDp);

			if (m_includeWeight)
				m_vtkWeight->SetValue(IDp,step.w)

				if (m_includeTime)
					m_vtkTime->SetValue(IDp,step.t);

			if(m_includeLength)
				m_vtkLength->SetValue(IDp,step.l);

			if (m_includeSteps)
				m_vtkLength->SetValue(IDp,++Nstep);

			++IDp;
			++i;
		}
	}

	// create cell array and assign it to the lines
	vtkCellArray* ca = vtkCellArray::New();
	ca->SetCells(Nt,IDps);
	m_vtkPD->SetLines(ca);

	Modified();
}

vtkPolyData* vtkFullMontePacketPositionTraceSetToPolyData::getPolyData() const
{
	return m_vtkPD;
}

void vtkFullMontePacketPositionTraceSetToPolyData::includeWeight(bool e)
{
	m_includeWeight=e;
}

void vtkFullMontePacketPositionTraceSetToPolyData::includeTime(bool e)
{
	m_includeTime=e;
}

void vtkFullMontePacketPositionTraceSetToPolyData::includeLength(bool e)
{
	m_includeLength=e;
}

void vtkFullMontePacketPositionTraceSetToPolyData::includeSteps(bool e)
{
	m_includeSteps=e;
}


