#pragma once
#include "graph.hpp"
#include "source.hpp"
#include "Material.hpp"

// simulator ID for database

#define DB_DEF_SIMULATOR 2

/** Problem definition - things which change the expected result.
 *
 */

class SimGeometry {
public:
	TetraMesh 			mesh;
	vector<Material> 	mats;
	vector<Source*>		sources;
	unsigned IDc;					// TODO: Probably shouldn't be included here since it's particular to the database backend
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
	unsigned Nhit_max=1000;
};

ostream& operator<<(ostream& os,const RunOptions& opts);
ostream& operator<<(ostream& os,const RunConfig& cfg);
ostream& operator<<(ostream& os,const SimGeometry& geom);
