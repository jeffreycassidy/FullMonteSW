#include "../fluencemap.hpp"

TetraMesh* exportMesh(PGConnection&,unsigned);
int exportMaterials(PGConnection&,unsigned,vector<Material>&);
vector<Material> exportMaterials(PGConnection& dbconn,unsigned IDc);

void runCase(PGConnection* dbconn,unsigned IDflight,unsigned IDsuite,unsigned caseorder);
void runCaseByID(PGConnection* dbconn,unsigned IDflight,unsigned IDcase,unsigned long long Nk);

// results
FluenceMapBase* exportResultSet(PGConnection* conn,unsigned IDr,unsigned dType,const TetraMesh* mesh=NULL);

vector<double> exportResultVector(PGConnection* conn,unsigned IDr,unsigned dType);


typedef struct {
	Oid			oid;
	double   	sum;
	unsigned 	Nbytes;
	unsigned 	dtype;
} ResultInfo;

typedef struct {
	unsigned long long Npkt;
	unsigned IDmesh;
	unsigned IDcase;
	unsigned IDrun;
	unsigned IDmaterialset;
	unsigned IDsourcegroup;

	vector<ResultInfo> results;
} RunInfo;


RunInfo getRunInfo(PGConnection* conn,unsigned IDr);
