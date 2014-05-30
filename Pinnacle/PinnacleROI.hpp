#pragma once
#include "PinnacleObject.hpp"
#include "PinnacleCurve.hpp"

#include <list>
#include <iostream>

using namespace std;

namespace Pinnacle {

class File;

class ROI : public Object
{
	list<Curve> curves;

public:

	bool validate(bool exc_=false) const;

	Curve* newcurve(){ curves.push_back(Curve()); return &curves.back(); };

	void printDetails() const;

	const list<Curve>& getCurves() const { return curves; }


	unsigned getNPoints() const {
		unsigned sum=0;
		for(const Curve& c : curves)
			sum += c.getNPoints();
		return sum;
	}

	void exportCurves(string fn) const;

	// exports curves in a Triangle .poly format
	void exportCurvesTriangle(string fn_,const list<unsigned>& curveIDs_) const;

	friend File;
};

}
