/*
 * EmpiricalCDF.cpp
 *
 *  Created on: Aug 11, 2016
 *      Author: jcassidy
 */



#include "EmpiricalCDF.hpp"

#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkPlotLine.h>
#include <vtkAxis.h>

#include <iostream>
#include <iomanip>

using namespace std;

/** Loads a CDF from triples (x, w, F(x))
 * Where x is the value, w is the sample weight (eg. area, volume, etc), and F(x) is Pr(X < x) in %
 * Sorted ascending
 **/

EmpiricalCDF<float,float> loadFromTextFile(const std::string& fn)
{
	ifstream is(fn.c_str());

	if (!is.good())
		throw std::logic_error("Failed to open file");

	vector<pair<float,float>> v;

	is >> skipws;

	while(!is.fail() && !is.eof())
	{
		float d, w, cw;
		is >> d >> w >> cw;
		is.ignore(1,'%');

		cw *= 0.01f;

		if (!is.fail())
			v.emplace_back(d,w);
	}

	cout << "Read " << v.size() << " elements" << endl;

	return EmpiricalCDF<float,float>(v);

}


vtkTable* convertToVTKTable(const EmpiricalCDF<float,float>& CDF)
{
	vtkTable* vtkT = vtkTable::New();

	vtkFloatArray* vtkArea = vtkFloatArray::New();
	vtkFloatArray* vtkDose = vtkFloatArray::New();

	vtkDose->SetName("Dose (J/cm2)");
	vtkDose->SetNumberOfTuples(CDF.dim());
	vtkT->AddColumn(vtkDose);

	vtkArea->SetName("Surface Area%");
	vtkArea->SetNumberOfTuples(CDF.dim());
	vtkT->AddColumn(vtkArea);

	vtkT->SetNumberOfRows(CDF.dim());

	// Replace F(x) with 1-F(x) to match DVH convention

	// CDF returns (x,F(x))

	for(unsigned i=0;i<CDF.dim();++i)
	{
		vtkDose->SetValue(i,CDF[i].first);
		vtkArea->SetValue(i,1.0f-CDF[i].second);
	}

	return vtkT;
}

#include <vtkContextView.h>
#include <vtkChartXY.h>
#include <vtkContextScene.h>
#include <vtkPlot.h>

vtkChartXY* drawDVH(vtkContextView* view,vtkTable* DVH)
{
	vtkChartXY* chart = vtkChartXY::New();

	view->GetScene()->AddItem(chart);

	vtkPlotLine* line = vtkPlotLine::SafeDownCast(chart->AddPlot(vtkChart::LINE));

	line->SetInputData(DVH,0,1);
	line->SetColor(255,0,0,255);
	line->SetWidth(1.0);

	vtkAxis* x = chart->GetAxis(vtkAxis::BOTTOM);
	vtkAxis* y = chart->GetAxis(vtkAxis::LEFT);

	x->SetBehavior(vtkAxis::FIXED);
	x->SetUnscaledRange(1e-5f,2e-2f);
	x->LogScaleOn();
	x->SetGridVisible(true);
	x->SetTitle("Dose (J/cm2)");

	y->SetBehavior(vtkAxis::FIXED);
	y->LogScaleOff();
	y->SetRange(0.0f,1.0f);
	y->SetGridVisible(true);
	y->SetTitle("Volume Fraction");

	chart->Update();

	return chart;
}

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkContext2D.h>

int main(int argc,char **argv)
{
	string fn("dsh_numbers.txt");

	if (argc >= 2)
		fn = argv[1];

	cout << "Loading DVH from " << fn << endl;

	EmpiricalCDF<float,float> CDF = loadFromTextFile(fn);

	CDF.print();

	vtkTable* vtkT = convertToVTKTable(CDF);

	cout << "Table has " << vtkT->GetNumberOfRows() << " rows and " << vtkT->GetNumberOfColumns() << " columns"<< endl;

	vtkRenderer* ren = vtkRenderer::New();

	vtkContextView* view = vtkContextView::New();
	view->GetRenderWindow()->SetSize(900,900);
//
//	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
//
//	vtkRenderWindow* renwin = view->GetRenderWindow();
//
//	renwin->AddRenderer(ren);
//
//	iren->SetRenderWindow(renwin);
//	iren->Initialize();
//	view->SetInteractor(iren);

	vtkChartXY* chart = drawDVH(view,vtkT);

	view->GetRenderWindow()->Render();
	view->GetInteractor()->Start();
}
