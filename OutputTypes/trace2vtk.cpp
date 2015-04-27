/*
 * trace2vtk.cpp
 *
 *  Created on: Apr 21, 2015
 *      Author: jcassidy
 */

#include "TracerStep.hpp"
#include <vector>
#include <array>

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

void writeVTKTraces(const std::string fn,const std::vector<TracerStep>& traces,const std::vector<unsigned>& L);

int main(int argc,char **argv)
{
	string fnpfx("../tracer.");
	string fnsfx(".bin");

	for(unsigned i=0;i<100;++i)
	{
		stringstream ss;
		ss << fnpfx << i << fnsfx;

		ifstream is(ss.str().c_str());

		cout << "Trace file " << ss.str() << endl;

		if (!is.good())
		{
			cout << "  Failed to open" << endl;
			break;
		}

		is.seekg(0,ios_base::end);
		size_t Nb=is.tellg();
		size_t Nbr=0;

		is.seekg(0,ios_base::beg);

		vector<TracerStep> S(Nb/sizeof(TracerStep));

		is.read((char*)S.data(),Nb);
		Nbr = is.gcount();

		cout << "  Length " << Nb << " bytes, read " << Nbr << " for a total " << S.size() << " records" << endl;

		vector<unsigned> launches;

		for(unsigned j=0;j<S.size();++j)
			if (S[j].event==TracerStep::Launch)
				launches.push_back(j);

		cout << "  Total " << launches.size() << " packets launched" << endl;

		stringstream oss;
		oss << fnpfx << i << ".vtk";
		cout << "  Writing to " << oss.str() << endl;
		writeVTKTraces(oss.str().c_str(),S,launches);
	}
}


#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdTypeArray.h>

#include <vtkUnstructuredGridWriter.h>
#include <vtkCell.h>
#include <vtkCellType.h>


void writeVTKTraces(const std::string fn,const std::vector<TracerStep>& traces,const std::vector<unsigned>& L)
{
	vtkPoints* P = vtkPoints::New();
	P->SetNumberOfPoints(traces.size());

	vtkIdTypeArray* ids = vtkIdTypeArray::New();
	vtkCellArray* ca = vtkCellArray::New();

	ids->SetNumberOfTuples(traces.size()+L.size());
	//ids->SetNumberOfTuples(traces.size());

	vtkIdType k=0;

	for(unsigned i=0;i<L.size();++i)
	{
		vtkIdType ub = i==L.size()-1 ? traces.size() : L[i+1];

		ids->SetTuple1(k++,ub-L[i]);
		for(unsigned j=L[i]; j<ub;++j)
		{
			ids->SetTuple1(k++,j);
			P->SetPoint(j,traces[j].pos.data());
		}
		cout << "trace size: " << ub-L[i] << endl;
	}

	ca->SetCells(L.size(),ids);

	vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
	ug->SetPoints(P);
	ug->SetCells(VTK_POLY_LINE,ca);

	vtkUnstructuredGridWriter *gw = vtkUnstructuredGridWriter::New();
	gw->SetInputData(ug);
	gw->SetFileName(fn.c_str());
	gw->Write();
	gw->Delete();
}
