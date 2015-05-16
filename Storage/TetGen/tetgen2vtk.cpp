///*
// * tetgen2vtk.cpp
// *
// *  Created on: Jan 31, 2015
// *      Author: jcassidy
// */
//
//#include "VTKHelpers.hpp"
//#include <iostream>
//
//#include <vector>
//#include <array>
//
//#include <boost/range.hpp>
//#include <boost/range/adaptor/map.hpp>
//#include <FullMonte/Storage/TetGen/TetGenReader.hpp>
//
//#include <vtkCellData.h>
//
//#include <vtkPoints.h>
//#include <vtkUnstructuredGrid.h>
//#include <vtkCellArray.h>
//#include <vtkUnsignedCharArray.h>
//
//using namespace std;
//
//int main(int argc,char **argv)
//{
//	string pfx;
//	if (argc < 2)
//		return (cerr << "Error: needs one argument (file name prefix)" << endl),0;
//	else
//		pfx = argv[1];
//
//	vector<array<double,3>> P;
//	vector<pair<array<unsigned,4>,unsigned>> T;
//
//	tie(P,T) = TetGen::load_node_ele(pfx);
//
//	vtkPoints *pts = vtkPoints::New();
//	boost::for_each(P, vtk_point_inserter(pts));
//
//	vtkCellArray *ca = vtkCellArray::New();
//	boost::for_each(T | boost::adaptors::map_keys, vtk_cell_inserter(ca));
//
//	vtkUnsignedCharArray *r = vtkUnsignedCharArray::New();
//	boost::for_each(T | boost::adaptors::map_values, [r](unsigned i){ r->InsertNextTuple1(i); });
//
//	vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
//
//	ug->SetPoints(pts);
//	ug->SetCells(VTK_TETRA,ca);
//	ug->GetCellData()->SetActiveScalars("regions");
//	ug->GetCellData()->SetScalars(r);
//
//	write_vtk_file(pfx+".vtk","Tetras refined by VTK",ug);
//
//}
