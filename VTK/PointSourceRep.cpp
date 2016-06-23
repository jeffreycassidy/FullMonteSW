/*
 * PointSourceRep.cpp
 *
 *  Created on: Nov 14, 2015
 *      Author: jcassidy
 */

#include <vtkCallbackCommand.h>
#include <vtkPointWidget.h>

#include <boost/range/algorithm.hpp>

#include <FullMonteSW/VTK/PointSourceRep.h>
#include <array>

#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkCellType.h>

#include <FullMonte/Geometry/Sources/PointSource.hpp>

using namespace std;



PointSourceRep::~PointSourceRep()
{}

vtkStandardNewMacro(PointSourceRep);

PointSourceRep::PointSourceRep() :
	m_pw(vtkPointWidget::New()),
	m_movecb(vtkCallbackCommand::New())
{
	// set up the widget for manipulating the point source
	m_pw->OutlineOn();
	m_pw->ZShadowsOn();
	m_pw->XShadowsOn();
	m_pw->YShadowsOn();
	//m_pw->SetEnabled(1);

	// setup callback object
	m_movecb->SetClientData(this);
	m_movecb->SetCallback(moveCallbackFunc);

	m_point = vtkPolyData::New();

	vtkPoints* p = vtkPoints::New();
	p->SetNumberOfPoints(1);
	p->SetPoint(0,0,0,0);

	vtkCellArray* ca = vtkCellArray::New();
	ca->InsertNextCell(1);
	ca->InsertCellPoint(0);

	m_point->SetPoints(p);
	m_point->SetVerts(ca);

	m_pw->AddObserver(vtkCommand::EndInteractionEvent,m_movecb);
}

void PointSourceRep::setPointSourceDescription(Source::PointSource *psd)
{
	m_psd=psd;
}

void PointSourceRep::setPointSourceDescription(const char* pStr)
{
	SwigPointerInfo info = readSwigPointer(pStr);
	setPointSourceDescription((Source::PointSource*)info.p);
}

// callback function called when the user moves the point
void PointSourceRep::moveCallbackFunc(vtkObject* caller,unsigned long eid,void *clientdata,void *calldata)
{
	assert(clientdata);
	assert(eid==vtkCommand::EndInteractionEvent);

	PointSourceRep *psr = static_cast<PointSourceRep*>(clientdata);
	psr->updateFromWidget();
}

std::array<float,3> PointSourceRep::position() const
{
	// get the point from the widget
	array<double,3> p;
	assert(m_pw);
	m_pw->GetPosition(p.data());
	array<float, 3> pf;

	// copy it for return
	boost::copy(p,pf.begin());
	return pf;
}

void PointSourceRep::position(array<float,3> pf)
{
	array<double,3> p;
	boost::copy(pf,p.begin());

	moveDescription(pf);
	moveWidget(pf);
	movePoint(pf);

	cout << "VTKPointSourceRep moved to " << p[0] << ' ' << p[1] << ' ' << p[2] << endl;
}

void PointSourceRep::moveDescription(const std::array<float,3> p)
{
	assert(m_psd);
	m_psd->position(p);
}

void PointSourceRep::moveWidget(const std::array<float,3> p)
{
	assert(m_pw);
	m_pw->SetPosition(p[0],p[1],p[2]);
}

void PointSourceRep::movePoint(const std::array<float,3> p)
{
	m_point->GetPoints()->SetPoint(0,p.data());
	m_point->Modified();
}

void PointSourceRep::updateFromWidget()
{
	array<float,3> newPos = position();

	m_point->GetPoints()->SetPoint(0,newPos.data());
	m_point->Modified();

	cout << "PointSourceRep updated to " << newPos[0] << ' ' << newPos[1] << ' ' << newPos[2] << endl;
}
