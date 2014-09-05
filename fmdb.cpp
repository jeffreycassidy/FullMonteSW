#include "fmdb.hpp"
#include "runresults.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>

PGFlight::PGFlight(PGConnection* dbconn_,const string& flightname_,const string& flightcomm_)
		: dbconn(dbconn_),
		  flightname(flightname_),
		  flightcomm(flightcomm_)
	{
		if(!dbconn)
			throw string("passed an invalid (NULL) PGConnection*");

		// get current date
		boost::posix_time::ptime t = boost::posix_time::second_clock::local_time();
		boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
		facet->format("%Y-%m-%d %H:%M:%S %z");

		std::stringstream ss;
		ss.imbue(std::locale(locale::classic(),facet));

		ss << t;

		// write to database
		PGConnection::ResultType res = dbconn->execParams("INSERT INTO flights(flightname,flightcomment,flightlaunch) VALUES ($1,$2,$3::timestamp) RETURNING flightid",
				boost::make_tuple(flightname,flightcomm,ss.str()));
	    unpackSinglePGRow(res,boost::tuples::tie(IDflight));

		// print to stdout
		cout << "Starting flight " << IDflight << endl;
		cout << "  ID: " << IDflight << endl;
		cout << "  Name: " << flightname << endl;
		cout << "  Comment: " << flightcomm << endl;
		cout << "  Launch time: " << ss.str() << endl;
	}

unsigned PGFlight::newRun(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc)
{
	unsigned IDrun;

    char hostname_buf[100];
    const char* login_name;

    if(gethostname(hostname_buf,99))
        throw string("Failed to get hostname");

    if(!(login_name=getlogin()))
    	throw std::string("Failed to get login name");
    string username(login_name);

    unsigned suiteid=0,caseorder=0;

    cout << "  hostname: " << hostname_buf << endl;
    cout << "  pid: " << hostname_buf << endl;
    cout << "  username: " << hostname_buf << endl;

    // start transaction (required for FK constraint on flights_map -> runs)
    dbconn->exec("BEGIN");

    cout << "Attempting to write" << endl;

	// create new run record, get run ID
	PGConnection::ResultType res = dbconn->execParams(
			"INSERT INTO runs(hostname,pid,username,path,args,simulator,suiteid,caseorder) VALUES "\
			"($1,$2,$3,$4,$5,$6,$7,$8) RETURNING runid;",
			boost::tuples::make_tuple(string(hostname_buf),getpid(),username,string(" "),string(" "),DB_DEF_SIMULATOR,suiteid,caseorder));
	unpackSinglePGRow(res,boost::tuples::tie(IDrun));

	cout << "Starting run " << IDrun << " as part of flight " << IDflight << endl;

	// add the rest of the info to the record (max 10 items in a boost::tuple
	// TODO: Change database backend to use std::tuple with no size limit
	res = dbconn->execParams(
			"UPDATE runs SET caseid=$2,packets=$3,threads=$4,randseed=$5,stepmax=$6,hitmax=$7,prwin=$8 WHERE runid=$1",
			boost::tuples::make_tuple(IDrun,IDc,cfg.Npackets,opts.Nthreads,opts.randseed,opts.Nstep_max,opts.Nhit_max,cfg.prwin));


	// link run to flight
	dbconn->execParams("INSERT INTO flights_map(flightid,runid) VALUES ($1,$2);",boost::tuples::make_tuple(IDflight,IDrun));

	// end transaction
    dbconn->exec("COMMIT");

    return IDrun;
}


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
    RunConfig   cfg;			// TODO: Doesn't read params from DB ?
    RunOptions	opts;			// TODO: Doesn't read params from DB ?

    PGConnection::ResultType res = dbconn->execParams("SELECT meshid,sourcegroupid,materialsetid,casename FROM cases " \
    		"WHERE caseid=$1;",boost::tuples::make_tuple(IDcase));

    unpackSinglePGRow(res,boost::tuples::tie(IDm,IDsg,IDmatset,casename));

    // load the mesh
    // TODO: Cache meshes that are used repeatedly
    geom.mesh = exportMeshByMeshID(*dbconn,IDm);

    // load sources and prepare within mesh
    geom.sources = exportSources(*dbconn,IDsg,1000000);

    cout << "Sources are: " << endl;
    for(const SourceDescription* sd : geom.sources)
    	cout << "  " << *sd << endl;

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

    string bpoints = dbconn.loadLargeObject(pdata_oid);
    string btetras = dbconn.loadLargeObject(tdata_oid);

    cout << "Read " << bpoints.size() << " bytes of points (" << bpoints.size()/3/sizeof(double) << " points)" << endl;
    cout << "Read " << btetras.size() << " bytes of tetras (" << btetras.size()/5/sizeof(unsigned) << " tetras)" << endl;

    if ((unsigned)bpoints.size()/3/sizeof(double) != (unsigned)Np || (unsigned)btetras.size()/5/sizeof(unsigned) != (unsigned)Nt)
    {
    	throw std::string("goofy stuff going on here");
        cerr << "Error: size mismatch!" << endl;
    }

    mesh.fromBinary(bpoints,btetras);

    return mesh;
}
