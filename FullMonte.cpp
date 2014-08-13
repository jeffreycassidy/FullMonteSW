#include "FullMonte.hpp"
#include <iostream>

/** Prints the contents of the RunOptions structure, using multiple lines.
 *
 */

ostream& operator<<(ostream& os,const RunOptions& opts)
{
	return os << "Run options" << endl
			<< "  Max steps: " << opts.Nstep_max << endl
			<< "  Max hits:  " << opts.Nhit_max << endl
			<< "  Threads:   " << opts.Nthreads << endl
			<< "  Timer interval: " << opts.timerinterval << endl
			<< "  Random seed:    " << opts.randseed << endl;
}


/** Prints the contents of the RunConfig structure, using multiple lines.
 *
 */

ostream& operator<<(ostream& os,const RunConfig& cfg)
{
	return os << "Run configuration" << endl
			<< "  wmin=" << scientific << cfg.wmin << endl
			<< "  prwin=" << cfg.prwin << endl
			<< "  Npackets=" << cfg.Npackets << endl;
}


/** Prints a summary of the SimGeometry structure, using multiple lines.
 * Provides only number of elements (tets, points, faces).
 *
 */

ostream& operator<<(ostream& os,const SimGeometry& geom)
{
	return os << "Geometry summary" << endl
			<< "  Tetrahedra: " << geom.mesh.getNt() << endl
			<< "  Points:     " << geom.mesh.getNp() << endl
			<< "  Faces:      " << geom.mesh.getNf() << endl;

}
