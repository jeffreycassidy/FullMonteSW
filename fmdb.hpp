#pragma once
#include "fm-postgres/fm-postgres.hpp"
#include "fluencemap.hpp"
#include "runresults.hpp"
#include <boost/timer/timer.hpp>
#include "FullMonte.hpp"

class PGFlight {
	PGConnection* dbconn=NULL;
	string flightname;
	string flightcomm;
	unsigned IDflight=0;

public:

	PGFlight(PGConnection* dbconn_,const string& flightname_="",const string& flightcomm_="");

	PGConnection* getConnection() const { return dbconn; }

	unsigned newRun(const SimGeometry&,const RunConfig&,const RunOptions&,unsigned IDc);
};



TetraMesh exportMeshByMeshID(PGConnection& dbconn, unsigned IDm);
std::tuple<SimGeometry,RunConfig,RunOptions> exportCaseByCaseID(PGConnection* dbconn,unsigned IDcase);
vector<Material> exportMaterialSetByID(PGConnection& dbconn,unsigned IDmatset);
