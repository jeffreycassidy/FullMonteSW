/*
 * PacketPositionTraceVTK.hpp
 *
 *  Created on: Nov 5, 2015
 *      Author: jcassidy
 */

#ifndef VTK_PACKETPOSITIONTRACEVTK_HPP_
#define VTK_PACKETPOSITIONTRACEVTK_HPP_

#include <FullMonteSW/OutputTypes/PacketPositionTrace.hpp>

#include <vtkObject.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>

#include <vtkPolyDataAlgorithm.h>

#include <vector>

/** vtkObject that produces vtkPolyLines from PacketPositionTraceToVTKPolyLine
 *
 */

class PacketPositionTraceToVTKPolyLine : public vtkPolyDataAlgorithm
{
public:

	////// The client interface

	void setTraceData(const std::vector<PacketPositionTrace>* p)
	{
		if (p != trace_)
		{
			trace_ = p;
			Modified();
		}
	}



	////// The VTK interface

	static PacketPositionTraceToVTKPolyLine *New();
	vtkTypeRevisionMacro(PacketPositionTraceToVTKPolyLine,vtkPolyDataAlgorithm);

	void PrintSelf(std::ostream& os,vtkIndent indent);


protected:
	PacketPositionTraceToVTKPolyLine();
	~PacketPositionTraceToVTKPolyLine();

	PacketPositionTraceToVTKPolyLine(const PacketPositionTraceToVTKPolyLine&) = delete;
	void operator=(const PacketPositionTraceToVTKPolyLine&) = delete;

	virtual int RequestData(vtkInformation* request,vtkInformationVector** inputVector,vtkInformationVector* outputVector) override;

private:
	const std::vector<PacketPositionTrace>* trace_=nullptr;
};





#endif /* VTK_PACKETPOSITIONTRACEVTK_HPP_ */
