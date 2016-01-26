/*
 * PacketPositionTraceVTK.cpp
 *
 *  Created on: Nov 5, 2015
 *      Author: jcassidy
 */

#include "PacketPositionTraceVTK.hpp"

#include <vtkObjectFactory.h>

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkCxxRevisionMacro(PacketPositionTraceToVTKPolyLine,"Rev 0.0");
vtkStandardNewMacro(PacketPositionTraceToVTKPolyLine);

PacketPositionTraceToVTKPolyLine::PacketPositionTraceToVTKPolyLine()
{
	SetNumberOfInputPorts(0);
	SetNumberOfOutputPorts(1);
}

PacketPositionTraceToVTKPolyLine::~PacketPositionTraceToVTKPolyLine()
{
}

void PacketPositionTraceToVTKPolyLine::PrintSelf(std::ostream& os,vtkIndent indent)
{
	os << "PacketPositionTraceToVTKPolyLine" << endl;
	Superclass::PrintSelf(os,indent);
}

int PacketPositionTraceToVTKPolyLine::RequestData(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector,vtkInformationVector* outputVector)
{
	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));


	//

	vtkPoints *pts = vtkPoints::New();
	vtkCellArray *ca = vtkCellArray::New();

	assert(trace_);

	for(const auto trace : *trace_)
	{
		ca->InsertNextCell(trace.steps().size());
		for(TraceStep ts : trace.steps())
		{
			vtkIdType IDp = pts->InsertNextPoint(ts.pos.data());
			ca->InsertCellPoint(IDp);
		}
	}

	output->SetPoints(pts);
	output->SetLines(ca);

	return 1;

}
