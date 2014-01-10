#include "fmdb.hpp"
#include "runresults.hpp"

#ifndef DB_DEF_SIMULATOR
#define DB_DEF_SIMULATOR 2
#endif

string username(getlogin());

unsigned db_startFlight(PGConnection* conn,const string& flightname,const string& flightcomment)
{
    unsigned runid;
    PGConnection::ResultType res = conn->execParams("INSERT INTO flights(flightname,flightcomment) VALUES ($1,$2) " \
        "RETURNING flightid;",boost::tuples::make_tuple(flightname,flightcomment));
    unpackSinglePGRow(res,boost::tuples::tie(runid));
    return runid;
}

unsigned db_startRun(PGConnection* dbconn,string path,string argstr,unsigned suiteid,unsigned caseorder,unsigned pid,
    unsigned flightid)
{
    const unsigned simulator = DB_DEF_SIMULATOR;
    unsigned runid;

    char hostname_buf[100];

    if(gethostname(hostname_buf,99))
        throw string("Failed to get hostname");

    dbconn->exec("BEGIN");

    PGConnection::ResultType res = dbconn->execParams(
        "INSERT INTO runs(hostname,pid,username,path,args,simulator,suiteid,caseorder) VALUES "\
            "($1,$2,$3,$4,$5,$6,$7,$8) RETURNING runid;",
        boost::tuples::make_tuple(string(hostname_buf),pid,username,path,argstr,simulator,suiteid,caseorder));
    unpackSinglePGRow(res,boost::tuples::tie(runid));

    dbconn->execParams("INSERT INTO flights_map(flightid,runid) VALUES ($1,$2);",boost::tuples::make_tuple(flightid,runid));

    dbconn->exec("COMMIT");

    return runid;
}

void db_finishRun(PGConnection* dbconn,unsigned runid,const RunResults& res)
{
    const char* log = res.log_stdout.getSize() > 0 ? (const char*)res.log_stdout.getPtr() : "";
    const char* err = res.log_stderr.getSize() > 0 ? (const char*)res.log_stderr.getPtr() : "";

    // write results to database
    dbconn->execParams("INSERT INTO runresults(runid,exitcode,t_wall,t_user,t_system,intersections,"\
        "launch) VALUES ($1,$2,$3,$4,$5,$6,$7)",boost::tuples::make_tuple(runid,
        res.exitcode,res.t_wall,res.t_user,res.t_system,res.Nintersection,res.Np));
    dbconn->execParams("UPDATE runresults SET steps=$2,tir=$3,scatter=$4,absorb=$5,fresnel=$6,exit=$7, " \
        "roulettewin=$8,refract=$9,die=$10 WHERE runid=$1;", boost::tuples::make_tuple(runid,
        res.Nabsorb,res.Ntir,res.Nscatter,res.Nabsorb,res.Nfresnel,res.Nexit,res.Nwin,res.Nrefr,res.Ndie));
	dbconn->execParams("INSERT INTO logtext(runid,stderr_text,stdout_text) VALUES ($1,$2,$3);",boost::tuples::make_tuple(
		runid,err,log));
}

