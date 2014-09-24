/** Construct a dose-volume histogram  */

// TODO: Make it work with a fluencemap as well (prob using iterators)

#include <boost/iterator/transform_iterator.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/count.hpp>

#include <algorithm>
#include <iostream>
#include <iomanip>

#include <string>
#include <fstream>

#include "DVH.hpp"

using namespace std;

DVH::DVH(const TetraMesh& M_,const vector<double>& fluence_,const vector<unsigned>& regions_)
{
	dvh.resize(M_.getNt());

	// insert all tetras into the DVH; assume zero variance for now
	for(unsigned IDt=1; IDt <= M_.getNt(); ++IDt)
		dvh[IDt-1] = DVHElement(regions_[IDt],M_.getTetraVolume(IDt),fluence_[IDt],0.0);

	sort();
}


void DVH::sort()
{
	// sort by region ID first, then by fluence
	std::sort(dvh.begin(),dvh.end(),DVHElement::DVHOrder);

	// find region boundaries
	unsigned IDr=0;
	for(vector<DVHElement>::const_iterator it_region_start=dvh.cbegin(), it_region_end = upper_bound(dvh.cbegin(),dvh.cend(),0,DVHElement::RegionOrder);
			it_region_start != dvh.cend();
			it_region_end = upper_bound(it_region_start=it_region_end,dvh.cend(),IDr,DVHElement::RegionOrder),++IDr)
		regions.emplace_back(dvh,it_region_start-dvh.cbegin(),it_region_end-dvh.cbegin());

	// do volume accumulation to get total area
	for(DVHRegion& R : regions)
		transform(R.cbegin(),R.cend(),R.begin(),DVHElement::DVHAccumulator(&R.V,&R.phi_bar));

	calcVolumeCDF();
}

/**
 * Precondition: dvh vector is ordered by (region,fluence)
 * Postcondition: dvh vector is ordered by (region,fluence) and volume_cdf values are valid
 */
void DVH::calcVolumeCDF()
{
	for(DVHRegion& R : regions)
		transform(R.begin(),R.end(),R.begin(),DVHElement::NormalizeVolume(R.V));
}


void DVH::DVHCheck() const
{
	cout << "Checking DVH" << endl;
	cout << "  Elements: " << dvh.size() << endl;
	cout << "  Regions:  " << regions.size() << endl;
	cout << "" << endl;
	cout << "Region stats: " << endl;

	unsigned i=0;

	vector<double> volumes;
	vector<unsigned> counts;
	double sum_last_volume=0;

	for(const DVHRegion& R : regions)
	{
		auto acc = for_each(
				boost::make_transform_iterator(R.cbegin(),std::mem_fn(&DVHElement::getVolume)),
				boost::make_transform_iterator(R.cend(),  std::mem_fn(&DVHElement::getVolume)),
				boost::accumulators::accumulator_set<double,boost::accumulators::features<boost::accumulators::tag::sum,boost::accumulators::tag::count>>());

		cout << "  " << setw(3) << i << ": " << R << ' ' << boost::accumulators::count(acc) <<
				" elements, total volume " << boost::accumulators::sum(acc) <<
				"; last volume_sum=" << (R.size() > 0 ? R.back().volume_sum : 0) <<
				"; mean fluence=" << R.getMeanFluence() << endl;
		sum_last_volume += (R.size() == 0 ? 0 : R.V);		// last element is idx_end-1
		volumes.push_back(boost::accumulators::sum(acc));
		counts.push_back(boost::accumulators::count(acc));
		++i;
	}

	auto total_v = for_each(volumes.begin(),volumes.end(),
			boost::accumulators::accumulator_set<double,boost::accumulators::features<boost::accumulators::tag::sum,boost::accumulators::tag::count>>());
	cout << "Total volume " << boost::accumulators::sum(total_v) << " over " << boost::accumulators::count(total_v) << " regions" << endl;
	cout << "Total volume (from summing last element of CDF): " << sum_last_volume << endl;

	auto total_count=for_each(counts.begin(),counts.end(),
			boost::accumulators::accumulator_set<unsigned,boost::accumulators::features<boost::accumulators::tag::sum,boost::accumulators::tag::count>>());
	cout << "Total elements " << boost::accumulators::sum(total_count) << " over " << boost::accumulators::count(total_count) << " regions" << endl;
}

void DVH::writeASCII(string fn,bool incl_var) const
{
	ofstream os(fn);

	os << "# created by DVH.cpp" << endl;
	os << "# first line: # of regions (Nr)" << endl;
	os << "# second line: space-separated list of number of elements in each region" << endl;
	os << "# subsequent lines: <fluence> <volume fraction>";
	if (incl_var)
		os << " <fluence variance=sigma2>";
	os << endl;
	os << regions.size() << endl;

	for(const auto& R : regions)
		os << R.size() << ' ';
	os << endl;

	if(!incl_var)
		for(const DVHElement& el : dvh)
			os << el.fluence << ' ' << el.volume_cdf << endl;
	else
		for(const DVHElement& el : dvh)
			os << el.fluence << ' ' << el.volume_cdf << ' ' << el.fluence_var << endl;
}
