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
%include "std_vector.i"

%{
#include <FullMonte/Geometry/SourceDescription.hpp>
%}

%template(sourcedescriptionvector) std::vector<SourceDescription*>;
%template(doublevector) std::vector<double>;

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

%typemap(out) std::pair<double,double>
{
	std::stringstream ss;
	ss << $1.first << ' ' << $1.second;
	Tcl_AppendResult(interp,ss.str().c_str(),NULL);
}

%typemap(out) std::array<double,3>
{
	std::array<double,3> a=$1;
	Tcl_Obj* const p[] = {
		Tcl_NewDoubleObj(a[0]),
		Tcl_NewDoubleObj(a[1]),
		Tcl_NewDoubleObj(a[2])};

	Tcl_Obj* obj = Tcl_NewListObj(3,p);
	Tcl_SetObjResult(interp,obj);
}


%typemap(in) std::array<double,3>
{
	std::array<double,3> a;
	std::stringstream ss(Tcl_GetString($input));
	for(unsigned i=0;i<3;++i)
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
VTK_TYPEMAP(vtkPointWidget)
VTK_TYPEMAP(vtkScalarsToColors)

%{

#include "FullMonteVTK.hpp"
#include <sstream>
#include <vtkTclUtil.h>

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
#include <vtkRenderWindowInteractor.h>
#include <vtkPointWidget.h>
#include <vtkLookupTable.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkCallbackCommand.h>

#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Geometry/SourceDescription.hpp>

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

	SourceDescription* getDescription() const { return bsd_; }

	void setCentre(const Point<3,double> p0){ assert(bsd_); bsd_->setCentre(p0); }
	void setRadius(double r){ assert(bsd_); bsd_->setRadius(r); }

	void Update();

	vtkActor* getActor(){ if (!actor_) Update(); return actor_; }
};

class VTKPointSourceRep {
	const VTKMeshRep& meshrep_;

	vtkPointWidget* pw_=nullptr;
	//vtkRenderWindowInteractor* iren_=nullptr;
	IsotropicPointSourceDescription *ips_=nullptr;

	// VTK callback function
	static void callbackFunc(vtkObject* caller,unsigned long eid,void *clientdata,void *calldata);

	vtkCallbackCommand* cb_=nullptr;

public:

	VTKPointSourceRep(const VTKMeshRep& mesh,const std::array<double,3> p0) :
		meshrep_(mesh), ips_(new IsotropicPointSourceDescription(Point<3,double>(p0))){ Update(); }

	// in this case, ips is a non-owning pointer
//	VTKPointSourceRep(const VTKMeshRep& mesh,vtkRenderWindowInteractor* iren_,IsotropicPointSourceDescription* ips) :
//		meshrep_(mesh),iren_(iren_),ips_(ips){ Update(); }

	~VTKPointSourceRep()
	{
		if(cb_)
			cb_->Delete();
		if(pw_)
			pw_->Delete();
	}

	// move point using means other than the interactor (eg. text entry) and update interactor appropriately
	void moveTo(std::array<double,3>);

	// gets the current position of the interactor
	std::array<double,3> getPosition() const;

	// not sure if Update is the right name here - really more of a create?
	void Update();

	IsotropicPointSourceDescription getSourceDescription() const { return *ips_; }

	vtkPointWidget* getWidget() { if(!pw_) Update(); return pw_; }
	//vtkActor*		getActor() 	{ if (!actor_) Update(); return actor_; }
};


class VTKLineSourceRep {
	const VTKMeshRep& meshrep_;
	LineSourceDescription* lsd_=nullptr;

	vtkActor* actor_=nullptr;

public:
	VTKLineSourceRep(const VTKMeshRep& mesh,const Point<3,double> p0,const Point<3,double> p1) :
		meshrep_(mesh), lsd_(new LineSourceDescription(p0,p1,1.0)){ Update(); }
#ifndef SWIG
	VTKLineSourceRep(const VTKMeshRep& mesh,LineSourceDescription* lsd) : meshrep_(mesh),lsd_(lsd)
		{ Update(); }
#endif

	SourceDescription* getDescription() const { return lsd_; }

	void endpoint(unsigned i,const Point<3,double> p){ assert(lsd_); lsd_->endpoint(i,p); Update(); }

	void Update();

	vtkActor* getActor(){ assert(lsd_); if (!actor_) Update(); return actor_; }
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

	vtkPolyData* getData() const { return pd_; }

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


class VTKVolumeFluenceRep {
	const VTKMeshRep& meshrep_;

	std::pair<double,double> phiRange_=std::make_pair(0,0);
	std::pair<double,double> lutRange_=std::make_pair(0,0);

	vtkActor* actor_=nullptr;
	vtkUnstructuredGrid* ug_=nullptr;
	vtkScalarBarActor *scale_=nullptr;

	vtkLookupTable* lut_=nullptr;

public:

	VTKVolumeFluenceRep(const VTKMeshRep& meshrep) : meshrep_(meshrep){}
	VTKVolumeFluenceRep(const VTKMeshRep& meshrep,const std::vector<double>& phi) : meshrep_(meshrep)
		{ Update(phi); }

	VTKVolumeFluenceRep(const VTKVolumeFluenceRep&) = delete;
	VTKVolumeFluenceRep(VTKVolumeFluenceRep&&) = default;

	~VTKVolumeFluenceRep(){
		if (actor_)
		{
			actor_->GetMapper()->Delete();
			actor_->Delete();
		}
		if (ug_)
			ug_->Delete();
		if (scale_)
			scale_->Delete();
	}

	void Update(const std::vector<double>& phi);

	vtkScalarBarActor* getScaleBar()
	{
		if (!actor_)
			Update(std::vector<double>());

		assert(actor_);

		if (!scale_)
		{
			scale_ = vtkScalarBarActor::New();
			scale_->SetLookupTable(getLookupTable());
			scale_->SetOrientationToVertical();
			scale_->SetTitle("Volume fluence (J/cm2)");
			scale_->DrawColorBarOn();
			//scale_->DrawTickLabelsOn();
		}
		return scale_;
	}

	vtkScalarsToColors* getLookupTable()
	{
		if (!lut_)
		{
			lut_=vtkLookupTable::New();
			//vtkDiscretizableColorTransferFunction* tlut_=vtkDiscretizableColorTransferFunction::New();
			lut_->SetNumberOfTableValues(256);
			lut_->SetRange(0,2000);
			lut_->Build();
			lut_->IndexedLookupOff();
		}

		return lut_;
	}

	std::pair<double,double> getRange() const { return phiRange_; }
	std::pair<double,double> getDisplayRange() const { return lutRange_; }

	vtkActor* getActor(){
		if (!actor_)
			Update(std::vector<double>());
		return actor_;
	}

	vtkUnstructuredGrid* getData() const { return ug_; }
};


class VTKVolumeSurfaceRep {
	const VTKMeshRep& meshrep_;
	std::pair<double,double> range_=make_pair(std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::quiet_NaN());

	// each pair contains (face ID, tetra ID)
	std::vector<std::pair<unsigned,unsigned>> surfaceElements_;

	vtkActor* actor_=nullptr;
	vtkPolyData* pd_=nullptr;
	vtkScalarBarActor *scale_=nullptr;

public:

	VTKVolumeSurfaceRep(const VTKMeshRep& meshrep,unsigned r0) : meshrep_(meshrep),
		surfaceElements_(meshrep.getMesh().getRegionBoundaryTrisAndTetras(r0,-1U))
			{ Update(std::vector<double>(meshrep.getMesh().getNt()+1,0.0)); }

	VTKVolumeSurfaceRep(const VTKMeshRep& meshrep,unsigned r0,unsigned r1) : meshrep_(meshrep),
	surfaceElements_(meshrep.getMesh().getRegionBoundaryTrisAndTetras(r0,r1))
		{ Update(std::vector<double>(meshrep.getMesh().getNt()+1,0.0)); }

//	VTKVolumeSurfaceRep(const VTKMeshRep& meshrep,const std::vector<double>& E,bool is_per_area) : meshrep_(meshrep)
//		{ Update(E,is_per_area); }

	VTKVolumeSurfaceRep(const VTKVolumeSurfaceRep&) = delete;
	VTKVolumeSurfaceRep(VTKVolumeSurfaceRep&&) = delete;

	~VTKVolumeSurfaceRep(){
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



	void setRange(double rmin,double rmax){
		range_=make_pair(rmin,rmax);
		if (scale_)
			scale_->SetLookupTable(actor_->GetMapper()->GetLookupTable());
	}

	// updates the rep to use fluence vector phi_v and region r
	void Update(const std::vector<double>& phi_v);

	vtkScalarBarActor* getScaleBar()
	{
		if (!actor_)
			Update(std::vector<double>());

		assert(actor_);

		if (!scale_)
		{
			scale_ = vtkScalarBarActor::New();
			scale_->SetLookupTable(actor_->GetMapper()->GetLookupTable());
			scale_->SetOrientationToVertical();
			scale_->SetTitle("Volume fluence (J/cm2)");
			scale_->DrawColorBarOn();
			scale_->DrawTickLabelsOn();
		}
		return scale_;
	}

	vtkActor* getActor(){
		if (!actor_)
			Update(std::vector<double>());
		return actor_;
	}

};



#endif /* FULLMONTEVTK_HPP_ */
