#include "fmdb.hpp"
#include "runresults.hpp"

//string username(getlogin());
///*
//unsigned db_startFlight(PGConnection* conn,const string& flightname,const string& flightcomment)
//{
//    unsigned IDflight;
//    PGConnection::ResultType res = conn->execParams("INSERT INTO flights(flightname,flightcomment) VALUES ($1,$2) " \
//        "RETURNING flightid;",boost::tuples::make_tuple(flightname,flightcomment));
//    unpackSinglePGRow(res,boost::tuples::tie(IDflight));
//    return IDflight;
//}
//
///** Marks the start of a single simulation run in the database.
// *
// * @param dbconn	Database connection
// * @param IDc		Case ID being run
// * @param IDflight	Flight ID to attach to
// * @param IDsuite	Suite ID being run (opt)
// * @param caseorder	Case order number (opt)
// */
//
//unsigned db_startRun(PGConnection* dbconn,const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned IDflight,unsigned suiteid,unsigned caseorder)
//{
//    const unsigned simulator = DB_DEF_SIMULATOR;
//    unsigned runid;
//
//    char hostname_buf[100];
//
//    if(gethostname(hostname_buf,99))
//        throw string("Failed to get hostname");
//
//    dbconn->exec("BEGIN");
//
//    PGConnection::ResultType res = dbconn->execParams(
//        "INSERT INTO runs(hostname,pid,username,path,args,simulator,suiteid,caseorder) VALUES "\
//            "($1,$2,$3,$4,$5,$6,$7,$8) RETURNING runid;",
//        boost::tuples::make_tuple(string(hostname_buf),getpid(),username,string(),string(),simulator,suiteid,caseorder));
//    unpackSinglePGRow(res,boost::tuples::tie(runid));
//
//    res = dbconn->execParams(
//            "UPDATE runs SET caseid=$2,packets=$3,threads=$4,randseed=$5,stepmax=$6,hitmax=$7,prwin=$8 WHERE runid=$1",
//            boost::tuples::make_tuple(runid,IDc,cfg.Npackets,opts.Nthreads,opts.randseed,opts.Nstep_max,opts.Nhit_max,cfg.prwin));
//
//    dbconn->execParams("INSERT INTO flights_map(flightid,runid) VALUES ($1,$2);",boost::tuples::make_tuple(IDflight,runid));
//
//    dbconn->exec("COMMIT");
//
//    return runid;
//}
//
//void db_finishRun(PGConnection* dbconn,unsigned runid,const RunResults& res)
//{
//    const char* log = res.log_stdout.getSize() > 0 ? (const char*)res.log_stdout.getPtr() : "";
//    const char* err = res.log_stderr.getSize() > 0 ? (const char*)res.log_stderr.getPtr() : "";
//
//    // write results to database
//    dbconn->execParams("INSERT INTO runresults(runid,exitcode,t_wall,t_user,t_system,intersections,"\
//        "launch) VALUES ($1,$2,$3,$4,$5,$6,$7)",boost::tuples::make_tuple(runid,
//        res.exitcode,res.t_wall,res.t_user,res.t_system,res.Nintersection,res.Np));
//    dbconn->execParams("UPDATE runresults SET steps=$2,tir=$3,scatter=$4,absorb=$5,fresnel=$6,exit=$7, " \
//        "roulettewin=$8,refract=$9,die=$10 WHERE runid=$1;", boost::tuples::make_tuple(runid,
//        res.Nabsorb,res.Ntir,res.Nscatter,res.Nabsorb,res.Nfresnel,res.Nexit,res.Nwin,res.Nrefr,res.Ndie));
//	dbconn->execParams("INSERT INTO logtext(runid,stderr_text,stdout_text) VALUES ($1,$2,$3);",boost::tuples::make_tuple(
//		runid,err,log));
//}

vector<Material> exportMaterialSetByID(PGConnection& dbconn,unsigned IDmatset)
{
	vector<Material> materials;

	const char qry[] = "SELECT materialindex,mu_a,mu_s,g,n FROM materials"
			"JOIN materialset_map ON materials.materialid=materialset_map.materialid "\
			"WHERE materials.materialsetid=$1 ORDER BY materialindex;";

    PGConnection::ResultType res;

    res = dbconn.execParams(qry,boost::tuples::make_tuple(IDmatset));

    unsigned matIdx;
    double mu_s,mu_a,g,n;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(matIdx,mu_a,mu_s,g,n),i);
        if (matIdx != (unsigned)i)
            cerr << "Materials are out of order!" << endl;
        materials.push_back(Material(mu_a,mu_s,g,n));
    }
    return materials;
}


// run based on a case ID
std::tuple<SimGeometry,RunConfig,RunOptions> exportCaseByCaseID(PGConnection* dbconn,unsigned IDcase)
{
    unsigned IDsg,IDmatset,IDm;
    string casename;

    SimGeometry geom;
    RunConfig   cfg;			// TODO: Doesn't read params from DB
    RunOptions	opts;			// TODO: Doesn't read params from DB

    PGConnection::ResultType res = dbconn->execParams("SELECT meshid,sourcegroupid,materialsetid,casename FROM cases " \
    		"WHERE caseid=$1;",boost::tuples::make_tuple(IDcase));

    unpackSinglePGRow(res,boost::tuples::tie(IDm,IDsg,IDmatset,casename));

    // load the mesh
    // TODO: Cache meshes that are used repeatedly
    geom.mesh = exportMeshByMeshID(*dbconn,IDm);

    //geom.mesh.fromBinary(dbconn->loadLargeObject(pdata_oid),dbconn->loadLargeObject(tdata_oid));

    // load sources and prepare within mesh
    vector<Source*> sources;
    exportSources(*dbconn,IDsg,sources,1000000);		// TODO: Why is this random constant here are 4th arg?
    geom.sources.push_back(sources.size() > 1 ? new SourceMulti(sources.begin(),sources.end()) : sources.front());
    geom.sources[0]->prepare(geom.mesh);

    // get the materials
    exportMaterials(*dbconn,IDmatset,geom.mats);

    return make_tuple(geom,cfg,opts);
}

TetraMesh exportMeshByMeshID(PGConnection& dbconn, unsigned IDm)
{
	const char qry[] = "SELECT npoints, ntetras, nfaces, pdata_oid, tdata_oid, fdata_oid, description FROM meshes WHERE meshid=$1;";
    TetraMesh mesh;

    PGConnection::ResultType res = dbconn.execParams(qry,boost::tuples::make_tuple(IDm));

    int Nr = PQntuples(res.get());
    cout << "Returned " << Nr << " records" << endl;

    // query results
    unsigned Nt,Np,Nf;
    Oid pdata_oid,tdata_oid,fdata_oid;
    string desc;

    unpackPGRow(res,boost::tuples::tie(Np,Nt,Nf,pdata_oid,tdata_oid,fdata_oid,desc),0);

    string name;

    cout << "First record (meshid " << IDm << ") has " << Np << " points, " << Nt << " tetras, and " << Nf << " faces" << endl;
    cout << "  pdata_oid=" << pdata_oid << ", tdata_oid=" << tdata_oid << ", fdata_oid=" << fdata_oid << endl;
    cout << "  name=" << name << " description=" << desc << endl;

    Blob bpoints = dbconn.loadLargeObject(pdata_oid);
    Blob btetras = dbconn.loadLargeObject(tdata_oid);

    cout << "Read " << bpoints.getSize() << " bytes of points (" << bpoints.getSize()/3/sizeof(double) << " points)" << endl;
    cout << "Read " << btetras.getSize() << " bytes of tetras (" << btetras.getSize()/5/sizeof(unsigned) << " tetras)" << endl;

    if ((unsigned)bpoints.getSize()/3/sizeof(double) != (unsigned)Np || (unsigned)btetras.getSize()/5/sizeof(unsigned) != (unsigned)Nt)
    {
    	throw std::string("goofy stuff going on here");
        cerr << "Error: size mismatch!" << endl;
    }

    mesh.fromBinary(bpoints,btetras);

    return mesh;
}
