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

	bool validate(bool exc_=false) const;

	Curve* newcurve(){ curves.push_back(Curve()); return &curves.back(); };

	void printDetails() const;

	/*
	iiterator<list<Curve>,decltype(mem_fn(Curve::size))> getCurvePoints() const { make_iiterator_adaptor
	}*/

	const list<Curve>& getCurves() const { return curves; }

	unsigned getNPoints() const {
		unsigned sum=0;
		for(const Curve& c : curves)
			sum += c.getNPoints();
		return sum;
	}

	//friend template<class OStream>OStream& operator<<(OStream&,const ROI&);
};

}
