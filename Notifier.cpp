#include "Notifier.hpp"

void OStreamObserver::runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned suiteid,unsigned caseorder)
{
	os << "Run started with options" << endl << cfg << endl << opts << endl;
	// moved from manager
	//    cout << "Launching sim: " << endl;
	//cout << geom << endl << cfg << endl << opts << endl;
}

void OStreamObserver::runfinish(boost::timer::cpu_times t,unsigned runid=0)
{
	os << "Run finished, elapsed time: " << format(t) << endl;
}

void OStreamObserver::flightstart(unsigned IDflight,string flightname,string flightcomment)
{
	os << "Flight started" << endl;
}

void OStreamObserver::flightfinish()
{
	os << "Flight finished" << endl;
}

unsigned PGObserver::createNewFlight(PGConnection* dbconn_,string flightname,string flightcomment)
{
	unsigned IDflight;
	PGConnection::ResultType res = dbconn_->execParams("INSERT INTO flights(flightname,flightcomment) VALUES ($1,$2) " \
			"RETURNING flightid;",boost::tuples::make_tuple(flightname,flightcomment));
	unpackSinglePGRow(res,boost::tuples::tie(IDflight));
	return IDflight;
}



void PGObserver::runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned IDflight,unsigned suiteid,unsigned caseorder)
{
    unsigned runid;

    char hostname_buf[100];
    const char* login_name;

    if(gethostname(hostname_buf,99))
        throw string("Failed to get hostname");

    dbconn->exec("BEGIN");

    if(!(login_name=getlogin()))
    	throw std::string("Failed to get login name");
    string username(login_name);

    PGConnection::ResultType res = dbconn->execParams(
        "INSERT INTO runs(hostname,pid,username,path,args,simulator,suiteid,caseorder) VALUES "\
            "($1,$2,$3,$4,$5,$6,$7,$8) RETURNING runid;",
        boost::tuples::make_tuple(string(hostname_buf),getpid(),username,string(),string(),DB_DEF_SIMULATOR,suiteid,caseorder));
    unpackSinglePGRow(res,boost::tuples::tie(runid));

    res = dbconn->execParams(
            "UPDATE runs SET caseid=$2,packets=$3,threads=$4,randseed=$5,stepmax=$6,hitmax=$7,prwin=$8 WHERE runid=$1",
            boost::tuples::make_tuple(runid,IDc,cfg.Npackets,opts.Nthreads,opts.randseed,opts.Nstep_max,opts.Nhit_max,cfg.prwin));

    dbconn->execParams("INSERT INTO flights_map(flightid,runid) VALUES ($1,$2);",boost::tuples::make_tuple(IDflight,runid));

    dbconn->exec("COMMIT");

    //return runid;
}

void PGObserver::runfinish(boost::timer::cpu_times t,unsigned runid)
{
    //const char* log = res.log_stdout.getSize() > 0 ? (const char*)res.log_stdout.getPtr() : "";
    //const char* err = res.log_stderr.getSize() > 0 ? (const char*)res.log_stderr.getPtr() : "";
	const char *err = "";
	const char *log = "";

    // write results to database
//    dbconn->execParams("INSERT INTO runresults(runid,exitcode,t_wall,t_user,t_system,intersections,"\
//        "launch) VALUES ($1,$2,$3,$4,$5,$6,$7)",boost::tuples::make_tuple(runid,
//        res.exitcode,res.t_wall,res.t_user,res.t_system,res.Nintersection,res.Np));
//    dbconn->execParams("UPDATE runresults SET steps=$2,tir=$3,scatter=$4,absorb=$5,fresnel=$6,exit=$7, " \
//        "roulettewin=$8,refract=$9,die=$10 WHERE runid=$1;", boost::tuples::make_tuple(runid,
//        res.Nabsorb,res.Ntir,res.Nscatter,res.Nabsorb,res.Nfresnel,res.Nexit,res.Nwin,res.Nrefr,res.Ndie));
//	dbconn->execParams("INSERT INTO logtext(runid,stderr_text,stdout_text) VALUES ($1,$2,$3);",boost::tuples::make_tuple(
//		runid,err,log));

	dbconn->execParams("INSERT INTO runresults(runid,exitcode,t_wall,t_user,t_system,launch) VALUES ($1,$2,$3,$4,$5)",
			boost::tuples::make_tuple(runid,0,(double)t.wall/1e9,(double)t.user/1e9,(double)t.system/1e9));
	    //dbconn->execParams("UPDATE runresults SET steps=$2,tir=$3,scatter=$4,absorb=$5,fresnel=$6,exit=$7, " \
	        //"roulettewin=$8,refract=$9,die=$10 WHERE runid=$1;", boost::tuples::make_tuple(runid,
	        //res.Nabsorb,res.Ntir,res.Nscatter,res.Nabsorb,res.Nfresnel,res.Nexit,res.Nwin,res.Nrefr,res.Ndie));
		dbconn->execParams("INSERT INTO logtext(runid,stderr_text,stdout_text) VALUES ($1,$2,$3);",boost::tuples::make_tuple(
			runid,err,log));
}
