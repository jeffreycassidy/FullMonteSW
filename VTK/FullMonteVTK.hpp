/*
 * FullMonteVTK.hpp
 *
 *  Created on: Mar 7, 2015
 *      Author: jcassidy
 */

#ifndef FULLMONTEVTK_HPP_
#define FULLMONTEVTK_HPP_

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkLegendBoxActor.h>
#include <vtkUnsignedShortArray.h>

#include <FullMonteSW/Geometry/TetraMesh.hpp>


class VTKMeshRep {
	// points definition shared by all representations of the dataset
	vtkPoints* P_=nullptr;

	vtkCellArray *tetras_=nullptr;

	vtkUnsignedShortArray *regions_=nullptr;

	const TetraMesh *mesh_=nullptr;

	std::vector<LegendEntry> legend_;

	void updatePoints();
	void updateRegions();
	void updateTetras();

public:
	VTKMeshRep(const TetraMesh* M);

	~VTKMeshRep(){
		if (P_) P_->Delete();
		if(tetras_) tetras_->Delete();
		if(regions_) regions_->Delete();
	}

	vtkUnstructuredGrid*	getMeshWithRegions() const;

	vtkPolyData*			getSubsetFaces(const std::vector<unsigned>& idx) const;
	vtkUnstructuredGrid*	getSubsetMesh(const std::vector<unsigned>& idx) const;

	// deal with legend

	void addLegendEntry(const LegendEntry& le){ legend_.push_back(le); }

	vtkLegendBoxActor*		getLegendActor(
			const std::array<float,2> ll=std::array<float,2>{0.75,0.05},
			const std::array<float,2> ur=std::array<float,2>{0.95,0.50}) const;
	void setLegend(const std::vector<LegendEntry>& legend){ legend_=legend; }

	vtkLookupTable* getRegionMapLUT() const;
};


#endif /* FULLMONTEVTK_HPP_ */
