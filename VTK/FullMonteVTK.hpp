/**
 * FullMonteVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */


#ifndef FULLMONTEVTK_HPP_
#define FULLMONTEVTK_HPP_

#ifdef SWIG

%module FullMonteVTK

%include "std_string.i"

// This line tells SWIG to pass the Tcl_Interp* whenever requested in wrapped function args, without consuming any input args
// Needed for VTK commands
%typemap(in,numinputs=0) Tcl_Interp* { $1 = interp; }

%typemap(in) std::array<float,2>
{
	std::array<float,2> a;
	std::stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<2;++i)
		ss >> a[i];
	$1 = a;
}

%typemap(in) Point<3,double>
{
	Point<3,double> p;
	std::stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
		ss >> p[i];
	$1 = p;
}

#define VTK_TYPEMAP(VTK_TYPE) %typemap(out) VTK_TYPE* { vtkTclGetObjectFromPointer(interp,$1,#VTK_TYPE); }

VTK_TYPEMAP(vtkUnstructuredGrid)
VTK_TYPEMAP(vtkPolyData)
VTK_TYPEMAP(vtkDataArray)
VTK_TYPEMAP(vtkLegendBoxActor)
VTK_TYPEMAP(vtkLookupTable)
VTK_TYPEMAP(vtkActor)
VTK_TYPEMAP(vtkScalarBarActor)

%{

#include "FullMonteVTK.hpp"
#include <sstream>
#include <vtk/vtkTclUtil.h>

%}

struct LegendEntry {
	std::string label;
	std::array<float,3> colour;
};

#endif


#ifndef SWIG

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkLegendBoxActor.h>
#include <vtkUnsignedShortArray.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkScalarBarActor.h>

#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/SourceDescription.hpp>

#endif // SWIG


class VTKMeshRep {
#ifndef SWIG
	// points definition shared by all representations of the dataset
	vtkPoints* P_=nullptr;

	vtkCellArray *tetras_=nullptr;

	vtkUnsignedShortArray *regions_=nullptr;

	const TetraMesh *mesh_=nullptr;

	std::vector<LegendEntry> legend_;

	void updatePoints();
	void updateRegions();
	void updateTetras();

	vtkActor* getSourceActor(const BallSourceDescription*) const;

#endif

public:
	VTKMeshRep(const TetraMesh* M);

	~VTKMeshRep(){
		if (P_) P_->Delete();
		if(tetras_) tetras_->Delete();
		if(regions_) regions_->Delete();
	}

	const TetraMesh& getMesh() const { assert(mesh_); return *mesh_; }
	vtkPoints* getPoints() const { assert(P_); return P_; }

	vtkUnstructuredGrid*	getMeshWithRegions() const;

	vtkPolyData*			getSubsetFaces(const std::vector<unsigned>& idx) const;
	vtkUnstructuredGrid*	getSubsetMesh(const std::vector<unsigned>& idx) const;

	vtkPolyData*			getSurfaceOfRegion(unsigned) const;

	// deal with legend

	void addLegendEntry(const LegendEntry& le){ legend_.push_back(le); }

#ifndef SWIG
	vtkLegendBoxActor*		getLegendActor(
			const std::array<float,2> ll=std::array<float,2>{0.75,0.05},
			const std::array<float,2> ur=std::array<float,2>{0.95,0.50}) const;
#else
	vtkLegendBoxActor*		getLegendActor(
			const std::array<float,2> ll,
			const std::array<float,2> ur) const;
#endif
	void setLegend(const std::vector<LegendEntry>& legend){ legend_=legend; }

	vtkActor* getSourceActor(const SourceDescription*) const;

	vtkLookupTable* getRegionMapLUT() const;
};

class VTKBallSourceRep {
	const VTKMeshRep& meshrep_;
	BallSourceDescription* bsd_=nullptr;
	std::vector<unsigned> T_;

	vtkUnstructuredGrid* ug_=nullptr;
	vtkActor* actor_=nullptr;

public:
	VTKBallSourceRep(const VTKMeshRep& mesh,const Point<3,double> p0,double r) :
		meshrep_(mesh), bsd_(new BallSourceDescription(p0,r,1.0)){ Update(); }
#ifndef SWIG
	VTKBallSourceRep(const VTKMeshRep& mesh,BallSourceDescription* bsd) : meshrep_(mesh),bsd_(bsd)
		{ Update(); }
#endif

	BallSourceDescription* getDescription() const { return bsd_; }

	void setCentre(const Point<3,double> p0){ assert(bsd_); bsd_->setCentre(p0); }
	void setRadius(double r){ assert(bsd_); bsd_->setRadius(r); }

	void Update();

	vtkActor* getActor(){ if (!actor_) Update(); return actor_; }
};

class VTKSurfaceFluenceRep {
	const VTKMeshRep& meshrep_;

	vtkActor* actor_=nullptr;
	vtkPolyData* pd_=nullptr;
	vtkScalarBarActor *scale_=nullptr;

public:

	VTKSurfaceFluenceRep(const VTKMeshRep& meshrep) : meshrep_(meshrep){}
	VTKSurfaceFluenceRep(const VTKMeshRep& meshrep,const std::vector<double>& E,bool is_per_area) : meshrep_(meshrep)
		{ Update(E,is_per_area); }

	VTKSurfaceFluenceRep(const VTKSurfaceFluenceRep&) = delete;
	VTKSurfaceFluenceRep(VTKSurfaceFluenceRep&&) = default;

	~VTKSurfaceFluenceRep(){
		if (actor_)
		{
			actor_->GetMapper()->Delete();
			actor_->Delete();
		}
		if (pd_)
			pd_->Delete();
		if (scale_)
			scale_->Delete();
	}

	void Update(const std::vector<double>& E,bool data_is_per_area);

	vtkScalarBarActor* getScaleBar()
	{
		if (!actor_)
			Update(std::vector<double>(),false);

		assert(actor_);

		if (!scale_)
		{
			scale_ = vtkScalarBarActor::New();
			scale_->SetLookupTable(actor_->GetMapper()->GetLookupTable());
			scale_->SetOrientationToVertical();
			scale_->SetTitle("Surface emittance (/mm2)");
			scale_->DrawColorBarOn();
			scale_->DrawTickLabelsOn();
		}
		return scale_;
	}

	vtkActor* getActor(){
		if (!actor_)
			Update(std::vector<double>(),false);
		return actor_;
	}
};


#endif /* FULLMONTEVTK_HPP_ */
