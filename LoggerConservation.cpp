#include "LoggerConservation.hpp"

/** Outputs a text summary of conservation statistics.
 * Difference output should be minimal.
 */

ostream& operator<<(ostream& os,const ConservationCounts& cc)
{
    double w_dispose = cc.w_absorb+cc.w_exit;
    double w_diff = w_dispose-cc.w_launch;

    os << "Energy conservation" << endl;
    os << "  Launched: " << cc.w_launch << endl;
    os << "  Disposed: " << w_dispose << endl;
    os << "    Absorbed " << cc.w_absorb << endl;
    os << "    Exited   " << cc.w_exit << endl;
    os << "  Difference: " << w_diff << " (" << 100.0*w_diff/cc.w_launch << "%)" << endl;
    os << endl;

    os << "Roulette difference " << cc.w_roulette-cc.w_die << endl;
    os << "  Died " << cc.w_die << endl;
    os << "  Added " << cc.w_roulette << endl;
    return os;
}

/// Add another ConservationCounts object to the current
ConservationCounts& ConservationCounts::operator+=(const ConservationCounts& lc)
{
	w_launch += lc.w_launch;
	w_absorb += lc.w_absorb;
	w_die    += lc.w_die;
	w_exit   += lc.w_exit;
	w_roulette += lc.w_roulette;
	return *this;
}
