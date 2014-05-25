#include "PinnacleROI.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

namespace Pinnacle {

void ROI::printDetails() const
{
	unsigned Ncurves=getprop<unsigned>("num_curve");
	cout << "ROI with " << curves.size() << " curves";

	if (Ncurves != curves.size())
		cout << " Mismatch: num_curves=" << Ncurves << endl;

	else
		cout << " (OK)" << endl;

	unsigned i=0;

	for(const Curve& c : curves)
	{
		cout << "    [" << setw(3) << i++ << "] ";
		c.printDetails();
	}
}

/** Returns true if the ROI is valid.
 *
 * @param exc_ Enable exceptions (defaults to false)
 *
 * @return Valid flag (true=good)
 */

bool ROI::validate(bool exc_) const
{
	unsigned Ncurves=getprop<unsigned>("num_curve");

	if(Ncurves != curves.size())
		return false;

	return true;
}

}

