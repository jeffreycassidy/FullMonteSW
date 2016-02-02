#pragma once
#include <FullMonte/Geometry/Sources/Base.hpp>
#include <FullMonte/Geometry/TetraMesh.hpp>
#include <FullMonte/Kernels/Software/Material.hpp>

// simulator ID for database

#define DB_DEF_SIMULATOR 2

/** Problem definition - things which change the expected result.
 *
 */

class SimGeometry {
public:
	TetraMesh 					mesh;
	vector<Material> 			mats;
	Source::Base*				source;
};

/** Run configuration - things that will change quality but not expected value.
 *
 * packet count
 * wmin
 * win probability
 */

class RunConfig {
public:
	unsigned long long Npackets=0;
	double wmin=1e-5;
	double prwin=0.1;

	RunConfig(){}
	RunConfig(const RunConfig& cfg_) = default;
};


 /** Run options (things that will not change the expected answer or output statistics)
 *
 * threads
 * hosts
 * seed
 * timer interval
 */

class RunOptions {
public:
	double timerinterval=1;
	unsigned randseed=100;
	unsigned Nthreads=8;
	unsigned Nstep_max=10000;
	unsigned Nhit_max=10000;
};

ostream& operator<<(ostream& os,const RunOptions& opts);
ostream& operator<<(ostream& os,const RunConfig& cfg);
ostream& operator<<(ostream& os,const SimGeometry& geom);
