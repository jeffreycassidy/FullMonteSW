#include "fm-postgres/fm-postgres.hpp"
#include "fluencemap.hpp"
#include "runresults.hpp"
#include <boost/timer/timer.hpp>
#include "FullMonte.hpp"

/* Data types
 * 		1		Surface integrated energy
 *		2		Volume integrated energy
 * 		3		Surface hit blob
 * 		4		Volume hit blob
 *
 */

TetraMesh exportMeshByMeshID(PGConnection& dbconn, unsigned IDm);
std::tuple<SimGeometry,RunConfig,RunOptions> exportCaseByCaseID(PGConnection* dbconn,unsigned IDcase);
vector<Material> exportMaterialSetByID(PGConnection& dbconn,unsigned IDmatset);

unsigned db_startFlight(PGConnection* conn,const string& flightname="",const string& flightcomment="");
unsigned db_startRun(PGConnection* dbconn,const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned IDflight,unsigned suiteid=0,unsigned caseorder=0);
void db_finishRun(PGConnection* dbconn,unsigned runid,const RunResults& res);

template<class T>unsigned   db_writeResult(PGConnection* conn,unsigned runid,const T& resultData);

template<class T>unsigned db_writeResult(PGConnection* conn,unsigned runid,const T& resultData)
{
    Blob b = resultData.toBinary();
    Oid oid = conn->createLargeObject(b);
    conn->execParams("INSERT INTO resultdata(runid,datatype,data_oid,total,bytesize) VALUES ($1,$2,$3,$4,$5)",
        boost::tuples::make_tuple(runid,T::dtype,oid,resultData.getTotalEnergy(),b.getSize()));
    return oid;
}
