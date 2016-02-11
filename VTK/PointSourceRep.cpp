/*
 * PointSourceRep.cpp
 *
 *  Created on: Nov 14, 2015
 *      Author: jcassidy
 */

#include <vtkCallbackCommand.h>
#include <vtkPointWidget.h>

#include <boost/range/algorithm.hpp>

#include <FullMonte/VTK/PointSourceRep.h>
#include <array>

#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkCellType.h>

#include <FullMonte/Geometry/Sources/PointSource.hpp>

using namespace std;


struct SwigPointerInfo {
	void* 								p;
	std::pair<const char*,const char*>	type;
};

template<typename T>T endianSwap(T i);

template<>uint64_t endianSwap(uint64_t i)
{
	i = (i >> 32) | (i << 32);
	i = ((i >> 16) & 0x0000ffff0000ffffULL) | ((i << 16) & 0xffff0000ffff0000ULL);
	i = ((i >> 8 ) & 0x00ff00ff00ff00ffULL) | ((i << 8)  & 0xff00ff00ff00ff00ULL);
	return i;
}


SwigPointerInfo readSwigPointer(const char *s)
{
	// format is _XXXXXXXXX_p_TTTTTT where X is pointer value (little-endian hex) and T is type
	uint64_t ptr;
	SwigPointerInfo info;

	assert(*s == '_');

	const char *pStart=s+1;
	const char *pEnd=s+1;
	for(pEnd=s+1; *pEnd != '\0' && *pEnd != '_';++pEnd){}

	sscanf(pStart,"%llx",&ptr);

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!
	info.p=reinterpret_cast<void*>(endianSwap(ptr));

	const char *tStart=pEnd, *tEnd=pEnd;

	assert(*(tStart++)=='_');
	assert(*(tStart++)=='p');
	assert(*(tStart++)=='_');

	for(tEnd=tStart+1; tEnd != '\0';++tEnd){}

	info.type = make_pair(tStart,tEnd);

	std::cout << "typeStr='" << std::string(tStart,tEnd) << "'" << std::endl;
	return info;
}

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
