#pragma once
#include "PinnacleObject.hpp"
#include "PinnacleCurve.hpp"

#include <list>
#include <iostream>

using namespace std;

namespace Pinnacle {

class ROI : public Object
{
	list<Curve> curves;
public:

	Curve* newcurve(){ curves.push_back(Curve()); return &curves.back(); };

	void printDetails() const { cout << "  ROI with " << curves.size() << " curves" << endl; }
};

}
