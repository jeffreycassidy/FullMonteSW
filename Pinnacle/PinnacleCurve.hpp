#pragma once
#include "PinnacleObject.hpp"

#include <vector>
#include <array>

namespace Pinnacle {

class Curve : public Object {
	vector<array<double,3>> points;
public:

	void addpoint(array<double,3> p){ points.push_back(p); }

};

}
