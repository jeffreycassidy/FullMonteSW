#include "fm-postgres/fm-postgres.hpp"
#include "fluencemap.hpp"
#include "runresults.hpp"
#include <boost/timer/timer.hpp>

unsigned db_startFlight(PGConnection* conn,const string& ="",const string& ="");
unsigned db_startRun(PGConnection* dbconn,string path,string argstr,unsigned suiteid,unsigned caseorder,unsigned pid,
    unsigned flightid);
void db_finishRun(PGConnection*,unsigned,const RunResults&);

template<class T>unsigned   db_writeResult(PGConnection* conn,unsigned runid,const T& resultData);

template<class T>unsigned db_writeResult(PGConnection* conn,unsigned runid,const T& resultData)
{
    Blob b = resultData.toBinary();
    Oid oid = conn->createLargeObject(b);
    conn->execParams("INSERT INTO resultdata(runid,datatype,data_oid,total,bytesize) VALUES ($1,$2,$3,$4,$5)",
        boost::tuples::make_tuple(runid,T::dtype,oid,resultData.getTotalEnergy(),b.getSize()));
    return oid;
}
