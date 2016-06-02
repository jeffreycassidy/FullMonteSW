/*
 * TetraMeshRegion.cpp
 *
 *  Created on: Nov 7, 2015
 *      Author: jcassidy
 */

#include <FullMonteSW/VTK/TetraMeshVTK.h>
#include <vtkObject.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkCellData.h>

#include "TetraMeshRegionsVTK.hpp"

#include <iostream>

using namespace std;

vtkCxxRevisionMacro(TetraMeshRegionsVTK,"Rev 0.0");
vtkStandardNewMacro(TetraMeshRegionsVTK);

TetraMeshRegionsVTK::TetraMeshRegionsVTK()
{
}

TetraMeshRegionsVTK::~TetraMeshRegionsVTK()
{
}


void TetraMeshRegionsVTK::PrintSelf(std::ostream& os,vtkIndent indent)
{
	os << "TetraMeshRegionsVTK :";
	Superclass::PrintSelf(os,indent);
}

int TetraMeshRegionsVTK::RequestData(vtkInformation* request,vtkInformationVector** inputVector,vtkInformationVector* outputVector)
{
	vtkInformation* inInfo = (*inputVector)->GetInformationObject(0);
	TetraMeshVTK* input = TetraMeshVTK::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

	vtkInformation *outInfo = outputVector->GetInformationObject(0);
	vtkUnstructuredGrid* output = vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

	output->SetPoints(input->m_vtkP);
	output->SetCells(VTK_TETRA,input->m_vtkT);

	output->GetCellData()->SetActiveScalars("regions");
	output->GetCellData()->SetScalars(input->m_vtkRegions);

	return 1;
}

