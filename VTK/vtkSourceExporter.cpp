/*
 * vtkSourceExporter.cpp
 *
 *  Created on: Oct 28, 2016
 *      Author: jcassidy
 */

#include "vtkSourceExporter.h"

#include <FullMonteSW/Geometry/TetraMesh.hpp>

#include <FullMonteSW/Geometry/Sources/Abstract.hpp>
#include <FullMonteSW/Geometry/Sources/Ball.hpp>
#include <FullMonteSW/Geometry/Sources/Line.hpp>
#include <FullMonteSW/Geometry/Sources/Point.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/Surface.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/Composite.hpp>

#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkAppendFilter.h>
#include <vtkMergeDataObjectFilter.h>
#include <vtkUnsignedShortArray.h>

#include <vtkObjectFactory.h>

#include <vtkSphereSource.h>
#include <vtkPolyData.h>


#include "SwigWrapping.hpp"


class ExportVisitor : public Source::Abstract::Visitor
{
public:
	ExportVisitor(const TetraMesh* M);
	~ExportVisitor();

	virtual void doVisit(Source::Line*) override;
	virtual void doVisit(Source::Point*) override;
	virtual void doVisit(Source::Volume*) override;
	virtual void doVisit(Source::SurfaceTri*) override;
	virtual void doVisit(Source::Surface*) override;
	virtual void doVisit(Source::PencilBeam*) override;
	virtual void doVisit(Source::Ball*) override;
	virtual void doVisit(Source::Abstract*) override;

	vtkUnstructuredGrid* 	output() const;

private:
	vtkPoints*				m_vtkP=nullptr;
	vtkUnsignedShortArray*	m_vtkS=nullptr;
	vtkDataObject*			m_labels=nullptr;

	vtkUnstructuredGrid* 	m_vtkUG=nullptr;

	unsigned				m_sourceCount=0;

	float					m_arrowLength=10.0f;		/// Length of arrow to add to pencil beam

	const TetraMesh*		m_mesh=nullptr;
};

ExportVisitor::ExportVisitor(const TetraMesh* M) :
		m_mesh(M)
{
	m_vtkP = vtkPoints::New();
	m_vtkS = vtkUnsignedShortArray::New();
		m_vtkS->SetName("Source Number");

	vtkFieldData* field = vtkFieldData::New();
		field->AddArray(m_vtkS);

	m_labels = vtkDataObject::New();
		m_labels->SetFieldData(field);

	m_vtkUG = vtkUnstructuredGrid::New();

	m_vtkUG->SetPoints(m_vtkP);
	m_vtkUG->GetCellData()->AddArray(m_vtkS);
}

ExportVisitor::~ExportVisitor()
{
	m_vtkUG->Delete();
}

void ExportVisitor::doVisit(Source::Line* L)
{
	vtkIdType id[2]{
		m_vtkP->InsertNextPoint(L->endpoint(0).data()),
		m_vtkP->InsertNextPoint(L->endpoint(1).data())
	};

	m_vtkUG->InsertNextCell(VTK_LINE,2,id);
	m_vtkS->InsertNextValue(m_sourceCount++);
}

void ExportVisitor::doVisit(Source::Point* P)
{
	vtkIdType id = m_vtkP->InsertNextPoint(P->position().data());
	m_vtkUG->InsertNextCell(VTK_VERTEX,1,&id);
	m_vtkS->InsertNextValue(m_sourceCount++);
}

void ExportVisitor::doVisit(Source::Volume* V)
{
	if(!m_mesh)
		throw std::logic_error("ExportVisitor::doVisit(Source::Volume*) called without valid mesh");

	vtkIdType ids[4];

	array<Point<3,double>,4> P = get(point_coords,*m_mesh,TetraMesh::TetraDescriptor(V->elementID()));

	for(unsigned i=0;i<4;++i)
		ids[i] = m_vtkP->InsertNextPoint(P[i].data());
	m_vtkUG->InsertNextCell(VTK_TETRA,4,ids);
	m_vtkS->InsertNextValue(m_sourceCount++);
}

void ExportVisitor::doVisit(Source::Ball* B)
{
	vtkSphereSource* S = vtkSphereSource::New();
	S->SetRadius(B->radius());

	const auto pf = B->centre();
	double p[3]{ pf[0], pf[1], pf[2] };
	S->SetCenter(p);
	S->SetThetaResolution(10);
	S->SetPhiResolution(10);
	S->Update();

	vtkUnsignedShortArray* Snum = vtkUnsignedShortArray::New();
	Snum->SetNumberOfTuples(S->GetOutput()->GetNumberOfCells());
	Snum->SetName("Source number");

	for(unsigned i=0;i<S->GetOutput()->GetNumberOfCells();++i)
		Snum->SetValue(i,m_sourceCount);
	m_sourceCount++;



	vtkFieldData* field = vtkFieldData::New();
		field->AddArray(Snum);

	vtkDataObject* obj = vtkDataObject::New();
		obj->SetFieldData(field);

	vtkMergeDataObjectFilter* merge = vtkMergeDataObjectFilter::New();
		merge->SetDataObjectInputData(obj);
		merge->SetInputConnection(S->GetOutputPort());
		merge->SetOutputFieldToCellDataField();
		merge->Update();

	vtkAppendFilter* AF = vtkAppendFilter::New();
		AF->AddInputData(m_vtkUG);
		AF->AddInputConnection(merge->GetOutputPort());
		AF->Update();

	m_vtkUG->ShallowCopy(AF->GetOutput());
	m_vtkP = m_vtkUG->GetPoints();
	m_vtkS = dynamic_cast<vtkUnsignedShortArray*>(m_vtkUG->GetCellData()->GetArray("Source number"));

	merge->Delete();
	AF->Delete();

	field->Delete();
	obj->Delete();

	S->Delete();
	Snum->Delete();
}

void ExportVisitor::doVisit(Source::Surface* S)
{
	if(!m_mesh)
		throw std::logic_error("ExportVisitor::doVisit(Source::Surface*) called without valid mesh");

	array<Point<3,double>,3> P = get(point_coords,*m_mesh,TetraMesh::FaceDescriptor(S->surfaceID()));

	vtkIdType id[3];
	for(unsigned i=0;i<3;++i)
		id[i] = m_vtkP->InsertNextPoint(P[i].data());

	m_vtkUG->InsertNextCell(VTK_TRIANGLE,3,id);
	m_vtkS->InsertNextValue(m_sourceCount++);
}
void ExportVisitor::doVisit(Source::SurfaceTri* S)
{
	if(!m_mesh)
		throw std::logic_error("ExportVisitor::doVisit(Source::SurfaceTri*) called without valid mesh");

	array<unsigned,3> IDps = S->triPointIDs();

	vtkIdType id[3];
	for(unsigned i=0;i<3;++i)
		id[i] = m_vtkP->InsertNextPoint(get(point_coords,*m_mesh,TetraMesh::PointDescriptor(IDps[i])).data());

	m_vtkUG->InsertNextCell(VTK_TRIANGLE,3,id);
	m_vtkS->InsertNextValue(m_sourceCount++);
}

void ExportVisitor::doVisit(Source::PencilBeam* PB)
{
	std::array<float,3> incidencePoint = PB->position();
	std::array<float,3> extensionPoint = PB->position()-PB->direction()*m_arrowLength;

	vtkIdType id[2]{
		m_vtkP->InsertNextPoint(extensionPoint.data()),
		m_vtkP->InsertNextPoint(incidencePoint.data())};

	m_vtkUG->InsertNextCell(VTK_LINE,2,id);
	m_vtkS->InsertNextValue(m_sourceCount++);
}

void ExportVisitor::doVisit(Source::Abstract* A)
{

	cout << "ERROR: Unknown source type in vtkSourceExport::ExportVisitor" << endl;
}

vtkUnstructuredGrid* ExportVisitor::output() const
{
	return m_vtkUG;
}

vtkSourceExporter::vtkSourceExporter()
{
}

vtkSourceExporter::~vtkSourceExporter()
{
}

void vtkSourceExporter::source(const char* s)
{
	SwigPointerInfo pInfo = readSwigPointer(s);
	string type(pInfo.type.first, pInfo.type.second-pInfo.type.first);

	if (pInfo.p)
	{
		if (type == "Source__Abstract")
			source(static_cast<Source::Abstract*>(pInfo.p));
		else if (type == "Source__Composite")
			source(static_cast<Source::Composite*>(pInfo.p));
//		else if (type == "")
//			source(static_cast<const SpatialMap<float>*>(pInfo.p));
		else
		{
			cout << "ERROR: SWIG pointer '" << s << "' is not a Source::Abstract, actually a " << type << endl;
			source(static_cast<Source::Abstract*>(nullptr));
		}
	}
	else
	{
		cout << "ERROR: Failed to convert SWIG pointer '" << s << "'" << endl;
		source(static_cast<Source::Abstract*>(nullptr));
	}
}

vtkUnstructuredGrid* vtkSourceExporter::output() const
{
	ExportVisitor V(m_mesh);
	V.visit(m_source);

	vtkUnstructuredGrid* o = V.output();

	vtkUnstructuredGrid* out = vtkUnstructuredGrid::New();
	out->ShallowCopy(o);

	return out;
}

Source::Abstract* vtkSourceExporter::source() const
{
	return m_source;
}

void vtkSourceExporter::source(Source::Abstract *S)
{
	m_source=S;
}

void vtkSourceExporter::mesh(const TetraMesh* M)
{
	m_mesh=M;
}

void vtkSourceExporter::mesh(const char* s)
{
	SwigPointerInfo pInfo = readSwigPointer(s);
	string type(pInfo.type.first, pInfo.type.second-pInfo.type.first);

	if (pInfo.p)
	{
		if (type == "TetraMesh")
			mesh(static_cast<const TetraMesh*>(pInfo.p));
		else
		{
			cout << "ERROR: SWIG pointer '" << s << "' is not a TetraMesh, actually a " << type << endl;
			mesh(static_cast<const TetraMesh*>(nullptr));
		}
	}
	else
	{
		cout << "ERROR: Failed to convert SWIG pointer '" << s << "'" << endl;
		mesh(static_cast<const TetraMesh*>(nullptr));
	}
}

vtkStandardNewMacro(vtkSourceExporter);
