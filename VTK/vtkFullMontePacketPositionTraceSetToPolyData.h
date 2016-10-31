/*
 * vtkFullMontePacketPositionTraceToPolyData.h
 *
 *  Created on: Sep 26, 2016
 *      Author: jcassidy
 */

#ifndef VTK_VTKFULLMONTEPACKETPOSITIONTRACESETTOPOLYDATA_H_
#define VTK_VTKFULLMONTEPACKETPOSITIONTRACESETTOPOLYDATA_H_

#include <vtkObject.h>

class vtkPolyData;
class PacketPositionTraceSet;
class vtkUnsignedShortArray;
class vtkFloatArray;

class vtkFullMontePacketPositionTraceSetToPolyData : public vtkObject
{
public:
	vtkTypeMacro(vtkFullMontePacketPositionTraceSetToPolyData,vtkObject)

	virtual ~vtkFullMontePacketPositionTraceSetToPolyData();

	static vtkFullMontePacketPositionTraceSetToPolyData* New();

	void source(const char* swigPtrString);
	void source(const PacketPositionTraceSet* traces);

	const PacketPositionTraceSet* source() const;

	void update();

	vtkPolyData* getPolyData() const;

	void includeWeight(bool e);
	void includeTime(bool e);
	void includeLength(bool e);
	void includeSteps(bool e);

protected:
	vtkFullMontePacketPositionTraceSetToPolyData();

private:
	bool						m_includeWeight=true;
	bool						m_includeTime=false;
	bool						m_includeLength=true;
	bool 						m_includeSteps=true;

	const PacketPositionTraceSet*	m_traces=nullptr;
	vtkPolyData*					m_vtkPD=nullptr;

	vtkFloatArray*					m_vtkWeight=nullptr;
	vtkFloatArray*					m_vtkTime=nullptr;
	vtkFloatArray*					m_vtkLength=nullptr;
	vtkUnsignedShortArray*			m_vtkStepCount=nullptr;
};


#endif /* VTK_VTKFULLMONTEPACKETPOSITIONTRACESETTOPOLYDATA_H_ */
