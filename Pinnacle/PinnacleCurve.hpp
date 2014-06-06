#pragma once
#include "PinnacleObject.hpp"

#include <vector>
#include <array>

namespace Pinnacle {

class File;

class Curve : public Object {
	vector<array<double,3>> points;
	unsigned sliceID=0;
public:

	void addpoint(array<double,3> p){ points.push_back(p); }

	unsigned size() const { return points.size(); }

	unsigned getSliceID() const { return sliceID; }

	void printDetails() const;

	const vector<array<double,3>>& getPoints() const { return points; }

	unsigned getNPoints() const { return points.size(); }

	friend File;
};

}
