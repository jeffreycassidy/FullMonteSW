/*
 * PointSourceRep.hpp
 *
 *  Created on: Nov 14, 2015
 *      Author: jcassidy
 */

#ifndef VTK_POINTSOURCEREP_H_
#define VTK_POINTSOURCEREP_H_

#include <array>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

class vtkObject;
class vtkPointWidget;
class vtkCallbackCommand;

namespace Source { class PointSource; };

class PointSourceRep : public vtkObject
{
public:
	vtkTypeRevisionMacro(PointSourceRep,vtkObject);
	static PointSourceRep* New();

	void setPointSourceDescription(Source::PointSource*);
	void setPointSourceDescription(const char*);

	// get/set position
	void position(std::array<float,3> newPos);
	std::array<float,3> position() const;

	// update from Point::Source*
	void updateFromWidget();


	////// VTK interface
	vtkPointWidget* 		pointWidget() 	const { return m_pw; }
	vtkPolyData*			point() 		const { return m_point; }

private:
	PointSourceRep();
	virtual ~PointSourceRep();

	void moveDescription(std::array<float,3>);
	void moveWidget(std::array<float,3>);
	void movePoint(std::array<float,3>);

	// Callback function for point source move
	static void moveCallbackFunc(vtkObject* caller,unsigned long eid,void *clientdata,void *calldata);

	vtkPointWidget*			m_pw = nullptr;
	vtkCallbackCommand*		m_movecb = nullptr;

	vtkPolyData*			m_point = nullptr;

	std::array<float,3>		m_pos = std::array<float,3>{0,0,0};
	Source::PointSource*	m_psd = nullptr;
};


#endif /* VTK_POINTSOURCEREP_H_ */
