#include "fluencemap.hpp"

TetraMesh* exportMesh(PGConnection&,unsigned);
//int exportSources(PGConnection&,unsigned,vector<Source*>&,long long Npacket=0);
int exportMaterials(PGConnection&,unsigned,vector<Material>&);
void runCase(PGConnection* dbconn,unsigned IDflight,unsigned IDsuite,unsigned caseorder);
void runCaseByID(PGConnection* dbconn,unsigned IDflight,unsigned IDcase,unsigned long long Nk);

// results
FluenceMapBase* exportResultSet(PGConnection* conn,unsigned IDr,unsigned dType,const TetraMesh* mesh=NULL);
