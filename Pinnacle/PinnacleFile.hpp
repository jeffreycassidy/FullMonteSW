#pragma once
#include "PinnacleObject.hpp"
#include "PinnacleROI.hpp"
#include "../../ece297/XMLWriter/XMLWriter.hpp"
#include <list>

#include <cxxabi.h>

using namespace std;

namespace Pinnacle {

class File : public Object {
	string fn;

	list<ROI> rois;

	map<double,unsigned> M;

	void buildSliceMap();

public:

	File(string fn_) : fn(fn_){ }

	bool read();

	ROI* newroi(){ rois.push_back(ROI()); return &rois.back(); }

	void printDetails() const;

	typedef iiterator<const list<ROI>,decltype(mem_fn(&ROI::getCurves))> curve_iiterator;
	typedef pair<curve_iiterator,curve_iiterator> curve_range;

	typedef iiterator<curve_range,decltype(mem_fn(&Curve::getPoints))> points_iiterator;
	typedef pair<points_iiterator,points_iiterator> points_range;

	curve_range getCurves() const {
		return make_iiterator_adaptor(rois,mem_fn(&ROI::getCurves));
	}

	points_range getPoints() const {
		auto r = getCurves();
		int status;
		cout << "Creating a points_range: typeid is " << abi::__cxa_demangle(typeid(r).name(),0,0,&status) << endl;
		return make_iiterator_adaptor(r,mem_fn(&Curve::getPoints));
	}


	typedef map<double,unsigned>::const_iterator slicemap_const_iterator;
	typedef pair<slicemap_const_iterator,slicemap_const_iterator> slicemap_const_range;

	const map<double,unsigned>& getSliceMap() const { return M; }
	slicemap_const_range getSliceMap(double z,double eps_=1e-6) const {
		slicemap_const_iterator b_it = M.lower_bound(z-eps_);
		slicemap_const_iterator e_it = M.upper_bound(z+eps_);
		return make_pair(b_it,e_it);
	}
	void printSliceMap() const;

	unsigned getNumberOfSlices() const {
		unsigned N=0;
		for(const pair<double,unsigned>& m : M)
			N=max(N,m.second);
		return N;
	}

	const list<ROI>& getROIs() const { return rois; }
	unsigned getNRois() const { return rois.size(); }
};

}
