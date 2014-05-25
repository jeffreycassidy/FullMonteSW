#pragma once
#include "PinnacleObject.hpp"
#include "PinnacleROI.hpp"
#include <list>

using namespace std;

namespace Pinnacle {

class File : public Object {
	string fn;

	list<ROI> rois;

public:

	File(string fn_) : fn(fn_){ read(); }

	bool read();

	ROI* newroi(){ rois.push_back(ROI()); return &rois.back(); }

	void printDetails() const;
};

}
