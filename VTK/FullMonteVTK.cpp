/*
 * FullMonteVTK.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#include "FullMonteVTK.hpp"

#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/algorithm.hpp>

#include <vtkCellData.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include <vtkLookupTable.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkFloatArray.h>

#include <limits>

using namespace std;

VTKMeshRep::VTKMeshRep(const TetraMesh* M)
	: mesh_(M)
{
	if (mesh_)
	{
		updatePoints();
		updateTetras();
		updateRegions();
	}
}

void VTKMeshRep::updatePoints()
{
	assert(mesh_);

	if(!P_)
		P_=vtkPoints::New();

	P_->SetNumberOfPoints(mesh_->getNp()+1);

	unsigned i=0;
	for(Point<3,double> p : mesh_->points())
		if (i==0)
			P_->SetPoint(i++,std::array<double,3>{ NAN, NAN, NAN}.data());
		else
			P_->SetPoint(i++,p.data());
}

void VTKMeshRep::updateTetras()
{
	assert(mesh_);
	assert(P_);

	size_t Nt=mesh_->getNt()+1;

	// Create tetra ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);
	ids->SetNumberOfTuples(5*Nt);

	unsigned j=0;
	for(TetraByPointID IDps : mesh_->getTetrasByPointID())
	{
		ids->SetTuple1(j++,4);
		for(unsigned k=0;k<4;++k)
			ids->SetTuple1(j++,IDps[k]);
	}

	// Form cell array
	if (!tetras_)
		tetras_ = vtkCellArray::New();
	tetras_->SetCells(Nt, ids);
}

vtkUnstructuredGrid* VTKMeshRep::getMeshWithRegions() const
{
	vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();
	ug->SetCells(VTK_TETRA,tetras_);
	ug->SetPoints(P_);
	ug->GetCellData()->SetActiveScalars("regions");
	ug->GetCellData()->SetScalars(regions_);

	return ug;
}

void VTKMeshRep::updateRegions()
{
	assert(mesh_);
	if (!regions_)
		regions_ = vtkUnsignedShortArray::New();

	regions_->SetNumberOfComponents(1);
	regions_->SetNumberOfTuples(mesh_->getNt()+1);

	for(unsigned i=0; i<=mesh_->getNt(); ++i)
		regions_->SetTuple1(i,mesh_->getMaterial(i));
}

// Returns a vtkUnstructuredGrid with a subset of the mesh in it (indices specified by idx)
vtkUnstructuredGrid* VTKMeshRep::getSubsetMesh(const std::vector<unsigned>& idx) const
{
	vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();

	// Create tetra ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);
	ids->SetNumberOfTuples(5*idx.size());

	unsigned j=0;
	for(TetraByPointID IDps : idx | boost::adaptors::transformed([this](unsigned i){ return this->mesh_->getTetraPointIDs(i); }))
	{
		ids->SetTuple1(j++,4);
		for(unsigned k=0;k<4;++k)
			ids->SetTuple1(j++,IDps[k]);
	}

	// Form cell array
	vtkCellArray *cells = vtkCellArray::New();
	cells->SetCells(idx.size(), ids);

	// Connect it to the data set
	ug->SetPoints(P_);
	ug->SetCells(VTK_TETRA,cells);

	return ug;
}


// Returns a vtkPolyData holding a subset of faces specified by the indices in idx
vtkPolyData* VTKMeshRep::getSubsetFaces(const std::vector<unsigned>& idx) const
{
	vtkPolyData *pd = vtkPolyData::New();

	// Create tetra ID array
	vtkIdTypeArray *ids = vtkIdTypeArray::New();
	ids->SetNumberOfComponents(1);
	ids->SetNumberOfTuples(4*idx.size());

	unsigned j=0;
	for(FaceByPointID IDps : idx | boost::adaptors::transformed([this](unsigned i){ return this->mesh_->getFacePointIDs(i); }))
	{
		ids->SetTuple1(j++,3);
		for(unsigned k=0;k<3;++k)
			ids->SetTuple1(j++,IDps[k]);
	}

	// Form cell array
	vtkCellArray *cells = vtkCellArray::New();
	cells->SetCells((vtkIdType)idx.size(), ids);

	// Connect it to the data set
	pd->SetPoints(P_);
	pd->SetPolys(cells);

	return pd;
}


// create an actor for the legend

vtkLegendBoxActor* VTKMeshRep::getLegendActor(const std::array<float,2> ll,
		const std::array<float,2> ur) const
{
	vtkLegendBoxActor* lba = vtkLegendBoxActor::New();

	lba->GetPositionCoordinate()->SetCoordinateSystemToView();
	lba->GetPosition2Coordinate()->SetCoordinateSystemToView();

	lba->GetPositionCoordinate()->SetValue(ll[0],ll[1]);
	lba->GetPosition2Coordinate()->SetValue(ur[0],ur[1]);

	cout << "  Drawing legend at (" << ll[0] << ',' << ll[1] << ")-(" << ur[0] << ',' << ur[1] << ')' << endl;
	cout << "  " << legend_.size() << " entries" << endl;


	// create box for legend symbol
	vtkIdType ids[] = { 0,1,2,3 };

	vtkPoints *pts = vtkPoints::New();
	pts->SetNumberOfPoints(4);
	pts->SetPoint(0,0.0,0.0,0.0);
	pts->SetPoint(1,2.0,0.0,0.0);
	pts->SetPoint(2,2.0,1.0,0.0);
	pts->SetPoint(3,0.0,1.0,0.0);

	vtkCellArray *ca = vtkCellArray::New();
	ca->InsertNextCell(4,ids);

	vtkPolyData *sym = vtkPolyData::New();
	sym->SetPoints(pts);
	sym->SetPolys(ca);


	// copy the legend entries
	lba->SetNumberOfEntries(legend_.size());
	for(const auto& le : legend_ | boost::adaptors::indexed(0U))
	{
		array<double,3> tcol;
		boost::copy(le.value().colour,tcol.data());
		lba->SetEntry(le.index(),sym,le.value().label.c_str(),tcol.data());
	}


	// set up box properties
	lba->SetBackgroundColor(0.5,0.5,0.5);
	lba->SetUseBackground(1);
	lba->SetBackgroundOpacity(0.5);

	return lba;
}

vtkLookupTable* VTKMeshRep::getRegionMapLUT() const
{
	vtkLookupTable* lut = vtkLookupTable::New();

	lut->SetNumberOfColors(legend_.size());
	lut->SetTableRange(0,legend_.size()-1);
	lut->IndexedLookupOn();
	lut->SetRange(0,legend_.size()-1);

	for(const auto& le : legend_ | boost::adaptors::indexed(0U))
	{
		std::array<double,4> rgba;
		boost::copy(le.value().colour, rgba.begin());
		rgba[3]=1.0;

		lut->SetTableValue(le.index(), rgba.data());
		lut->SetAnnotation(le.index(), le.value().label);
	}

	return lut;
}

vtkActor* VTKMeshRep::getSourceActor(const SourceDescription* sd) const
{
	if (const BallSourceDescription* bsd = dynamic_cast<const BallSourceDescription*>(sd))
		return getSourceActor(bsd);
	assert(!"Unrecognized source in getSourceActor");
}

vtkActor* VTKMeshRep::getSourceActor(const BallSourceDescription* bsd) const
{
	assert(mesh_);
	Point<3,double> p0 = bsd->getCentre();
	float r = bsd->getRadius();

	std::vector<unsigned> T = mesh_->tetras_close_to(p0,r);
	cout << "There are " << T.size() << " tetras in the source" << endl;

	vtkUnstructuredGrid* ug = getSubsetMesh(T);

	vtkDataSetMapper *map = vtkDataSetMapper::New();
	map->SetInputData(ug);

	vtkActor *actor = vtkActor::New();
	actor->SetMapper(map);

	return actor;
}

vtkPolyData* VTKMeshRep::getSurfaceOfRegion(unsigned r) const
{
	assert(mesh_);
	vector<unsigned> tri = mesh_->getRegionBoundaryTris(r);

	return getSubsetFaces(tri);
}




void VTKBallSourceRep::Update()
{
	assert(bsd_);
	T_ = meshrep_.getMesh().tetras_close_to(bsd_->getCentre(),bsd_->getRadius());

	if (!actor_)
	{
		actor_ = vtkActor::New();

		vtkDataSetMapper* mapper = vtkDataSetMapper::New();
		actor_->SetMapper(mapper);
	}
	else
		actor_->GetMapper()->GetInput()->Delete();

	dynamic_cast<vtkDataSetMapper*>(actor_->GetMapper())->SetInputData(meshrep_.getSubsetMesh(T_));
}

void VTKSurfaceFluenceRep::Update(const std::vector<double>& E,bool is_per_area)
{
	if (!pd_)
	{
		pd_ = vtkPolyData::New();
		pd_->SetPoints(meshrep_.getPoints());

		pd_->SetPolys(vtkCellArray::New());

		pd_->GetCellData()->SetActiveScalars("fluence");
		pd_->GetCellData()->SetScalars(vtkFloatArray::New());
	}

	vtkFloatArray *phi_field = vtkFloatArray::SafeDownCast(pd_->GetCellData()->GetScalars());
	assert(phi_field);
	phi_field->Initialize();

	vtkCellArray *ca = pd_->GetPolys();
	ca->Initialize();

	double phi_min=std::numeric_limits<double>::infinity();
	double phi_max=-phi_min;
	double phi;

	unsigned nnz=0;
	for(unsigned i=0;i<E.size(); ++i)
	{
		if (E[i] > 0.0)
		{
			++nnz;
			std::array<unsigned,3>  IDps = meshrep_.getMesh().getFacePointIDs(i);
			std::array<vtkIdType,3> IDps_vtk;
			boost::copy(IDps, IDps_vtk.begin());
			ca->InsertNextCell(3,IDps_vtk.data());
			phi = is_per_area ? E[i] : E[i]/meshrep_.getMesh().getFaceArea(i);
			phi_min = min(phi_min,phi);
			phi_max = max(phi_max,phi);
			phi_field->InsertNextTuple1(phi);
		}
	}

	pd_->SetPolys(ca);
	pd_->SetPoints(meshrep_.getPoints());

//	cout << "INFO: Updated vector using the " << nnz << '/' << E.size() << " nonzero elements" << endl;
//	cout << "  Container reports " << pd_->GetNumberOfPolys() << " polys, " << pd_->GetNumberOfPoints() << " points" << endl;


	if (!actor_)
	{
		actor_ = vtkActor::New();

		vtkPolyDataMapper* mapper = vtkPolyDataMapper::New();
		actor_->SetMapper(mapper);
	}

	vtkPolyDataMapper *mapper = vtkPolyDataMapper::SafeDownCast(actor_->GetMapper());
	assert(mapper);

	mapper->SetInputData(pd_);

	if (!mapper->GetLookupTable())
		mapper->CreateDefaultLookupTable();

	mapper->ScalarVisibilityOn();
	mapper->SetScalarRange(0,phi_max);
	mapper->GetLookupTable()->SetRange(0,phi_max);

	actor_->GetMapper()->Update();
}
