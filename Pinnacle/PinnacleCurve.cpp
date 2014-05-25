#include "PinnacleCurve.hpp"

#include <iostream>

using namespace std;

namespace Pinnacle {

void Curve::printDetails() const {
	cout << points.size() << " points" << endl;
}

}
