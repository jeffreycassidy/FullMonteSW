#include "RegionSet.hpp"

void RegionSet::print(std::ostream& os) const
{
	unsigned i=0;
	for(unsigned r : R)
		os << "  " << i++ << ": " << r << std::endl;
}

std::vector<unsigned> RegionSet::getRegionElementCounts() const
{
	std::vector<unsigned> c(Nr,0);
	for(unsigned r : R)
		c[r]++;
	return c;
}
