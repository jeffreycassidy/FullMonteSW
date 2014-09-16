#include "TetraMeshBaseVTK.hpp"

vtkPoints* getVTKPoints(const TetraMeshBase& M)
{
	const vector<Point<3,double>>& P=M.points();
	vtkPoints* vtkp = vtkPoints::New();

	vtkp->SetNumberOfPoints(P.size());
	unsigned i=0;
	for(const Point<3,double>& p : P)
		vtkp->SetPoint(i++, p.data());
	return vtkp;
}

vtkCellArray* getVTKTetras(const TetraMeshBase& M)
{
	const vector<TetraByPointID>& T_p=M.tetrasByID();

	vtkIdTypeArray *cells = vtkIdTypeArray::New();
	cells->SetNumberOfComponents(1);
	cells->SetNumberOfTuples(5*T_p.size());

	vtkIdType idx=0;
	for(TetraByPointID IDps : T_p)
	{
		cells->SetTuple1(idx++,4);
		cells->SetTuple1(idx++,IDps[0]);
		cells->SetTuple1(idx++,IDps[1]);
		cells->SetTuple1(idx++,IDps[2]);
		cells->SetTuple1(idx++,IDps[3]);
	}

	vtkCellArray* vtkca = vtkCellArray::New();
	vtkca->SetCells(T_p.size(),cells);
	return vtkca;
}

vtkUnstructuredGrid* getVTKTetraMesh(const TetraMeshBase& M)
{
	vtkUnstructuredGrid* ds = vtkUnstructuredGrid::New();

	// Cells
	vtkCellArray *vtkcells = getVTKTetras(M);
	ds->SetCells(VTK_TETRA,vtkcells);

	// Points
	vtkPoints* vtkpoints = getVTKPoints(M);
	ds->SetPoints(vtkpoints);

	// Data
	//vtkUnsignedShortArray* vtkregions = vtkUnsignedShortArray::New();
	//vtkregions->SetNumberOfComponents(1);
	//vtkregions->SetNumberOfTuples(M.getNt());

	//for(unsigned i=0;i<M.getNt();++i)
	//	vtkregions->SetTuple1(i,M.getMaterial(i+1));

	//ds->GetCellData()->SetScalars(vtkregions);

	return ds;
}

