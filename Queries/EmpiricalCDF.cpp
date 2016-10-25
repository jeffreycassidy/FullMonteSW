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

	return EmpiricalCDF<float,float>(v);

}
