#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <boost/static_assert.hpp>

#include <boost/range/adaptor/indexed.hpp>
#include "fm-postgres.hpp"
#include "fmdbexportcase.hpp"

#include "io_timos.hpp"
#include "SourceDescription.hpp"
#include "graph.hpp"

using namespace std;
vector<SourceDescription*> exportSources(PGConnection& dbconn,unsigned IDsourcegroup,long long Npacket)
{
	vector<SourceDescription*> sources;

    // Isotropic point sources
    stringstream qry;
    qry << "SELECT default_w,pos FROM sources ";
    qry << "JOIN sources_ips ON sources_ips.sourceid=sources.sourceid ";
    qry << "WHERE sourcegroupid=$1;";

    PGConnection::ResultType res;

    res = dbconn.execParams(qry.str().c_str(),boost::tuples::make_tuple(IDsourcegroup));

    double default_w;
    Point<3,double> p;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(default_w,p),i);
        IsotropicPointSourceDescription* s = new IsotropicPointSourceDescription(p,default_w);
        sources.push_back(s);
    }

    // Volume sources
    qry.str(string());
    qry << "SELECT default_w,tetraid FROM sources  ";
    qry << "JOIN sources_vol ON sources_vol.sourceid=sources.sourceid ";
    qry << "WHERE sourcegroupid=$1 ORDER BY sources_vol.tetraid;";

    res = dbconn.execParams(qry.str().c_str(),boost::tuples::make_tuple(IDsourcegroup));

    unsigned IDt,IDf;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(default_w,IDt),i);
        VolumeSourceDescription* s = new VolumeSourceDescription(IDt,default_w);
        sources.push_back(s);
    }

    // Triangle (face) sources
    qry.str(string());
    qry << "SELECT default_w,faceid,tetraid,pointids FROM sources ";
    qry << "JOIN sources_tri ON sources_tri.sourceid=sources.sourceid ";
    qry << "WHERE sourcegroupid=$1 ORDER BY sources_tri.tetraid;";

    res = dbconn.execParams(qry.str().c_str(),boost::tuples::make_tuple(IDsourcegroup));

    FaceByPointID IDps;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(default_w,IDf,IDt,IDps),i);
        FaceSourceDescription* s = new FaceSourceDescription(IDps,default_w);
        sources.push_back(s);
    }

    // Pencil beam sources
    qry.str(string());
    qry << "SELECT default_w,pos,dir,tetraid FROM sources ";
    qry << "JOIN sources_pb ON sources_pb.sourceid=sources.sourceid ";
    qry << "WHERE sourcegroupid=$1 ORDER BY sources_pb.tetraid;";

    res = dbconn.execParams(qry.str().c_str(),boost::tuples::make_tuple(IDsourcegroup));

    Point<3,double> dir;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(default_w,p,dir,IDt),i);
        PencilBeamSourceDescription* s = new PencilBeamSourceDescription(p,UnitVector<3,double>(dir),default_w,IDt);
        sources.push_back(s);
    }


    // Pencil beam sources
    qry.str(string());
    qry << "SELECT default_w,p0,p1 FROM sources ";
    qry << "JOIN sources_line ON sources_line.sourceid=sources.sourceid ";
    qry << "WHERE sourcegroupid=$1 ORDER BY sources.sourceid;";

    res = dbconn.execParams(qry.str().c_str(),boost::tuples::make_tuple(IDsourcegroup));

    Point<3,double> p0,p1;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(default_w,p0,p1),i);
        LineSourceDescription* s = new LineSourceDescription(p0,p1,default_w);
        sources.push_back(s);
    }

    return sources;
}

// TODO: Deprecate this, just return a vector (don't be silly)!
int exportMaterials(PGConnection& dbconn,unsigned IDc,vector<Material>& materials)
{
    materials.clear();

    stringstream qry;
    qry << "SELECT materialindex,mu_a,mu_s,g,n FROM materials JOIN materialset_map ON materials.materialid=materialset_map.materialid ";
    qry << "JOIN cases ON cases.materialsetid=materialset_map.materialsetid ";
    qry << "WHERE cases.caseid=$1 ORDER BY materialindex;";

    PGConnection::ResultType res;

    res = dbconn.execParams(qry.str().c_str(),boost::tuples::make_tuple(IDc));

    unsigned matIdx;
    double mu_s,mu_a,g,n;

    for(int i=0;i<PQntuples(res.get());++i)
    {
        unpackPGRow(res,boost::tuples::tie(matIdx,mu_a,mu_s,g,n),i);
        if (matIdx != (unsigned)i)
            cerr << "Materials are out of order!" << endl;
        materials.push_back(Material(mu_a,mu_s,g,n));
    }
    return materials.size();
}

vector<Material> exportMaterials(PGConnection& dbconn,unsigned IDc)
{
	vector<Material> mats;
	exportMaterials(dbconn,IDc,mats);
	return mats;
}

// TODO: Deprecated; move into fmdb in upper folder
TetraMesh* exportMesh(PGConnection& dbconn, unsigned IDc)
{
    stringstream qrystr;

    qrystr << "SELECT cases.meshid, npoints, ntetras, nfaces, pdata_oid, tdata_oid, fdata_oid, description FROM cases JOIN meshes ON cases.meshid=meshes.meshid WHERE caseid=$1;";

    PGConnection::ResultType res = dbconn.execParams(qrystr.str().c_str(),boost::tuples::make_tuple(IDc));

    int Nr = PQntuples(res.get());
    cout << "Returned " << Nr << " records" << endl;

    // query results
    unsigned meshid,Nt,Np,Nf;
    Oid pdata_oid,tdata_oid,fdata_oid;
    string desc;

    unpackPGRow(res,boost::tuples::tie(meshid,Np,Nt,Nf,pdata_oid,tdata_oid,fdata_oid,desc),0);

    string name;

    cout << "First record (meshid " << meshid << ") has " << Np << " points, " << Nt << " tetras, and " << Nf << " faces" << endl;
    cout << "  pdata_oid=" << pdata_oid << ", tdata_oid=" << tdata_oid << ", fdata_oid=" << fdata_oid << endl;
    cout << "  name=" << name << " description=" << desc << endl;

    string bpoints = dbconn.loadLargeObject(pdata_oid);
    string btetras = dbconn.loadLargeObject(tdata_oid);

    cout << "Read " << bpoints.size() << " bytes of points (" << bpoints.size()/3/sizeof(double) << " points)" << endl;
    cout << "Read " << btetras.size() << " bytes of tetras (" << btetras.size()/5/sizeof(unsigned) << " tetras)" << endl;

    if ((unsigned)bpoints.size()/3/sizeof(double) != (unsigned)Np || (unsigned)btetras.size()/5/sizeof(unsigned) != (unsigned)Nt)
    {
        cerr << "Error: size mismatch!" << endl;
        return NULL;
    }

    TetraMesh *m = new TetraMesh;
    m->fromBinary(bpoints,btetras);

    return m;
}


/** Exports a result dataset
 *
 * @param conn Database connection pointer
 * @param IDr	Run ID to export
 * @param dType	Datatype (1=surface, 2=volume)
 * @param mesh	Associated mesh
 */

FluenceMapBase* exportResultSet(PGConnection* conn,unsigned IDr,unsigned dType,const TetraMesh* mesh)
{
    unsigned long long packets;
    FluenceMapBase* data;

    Oid data_oid;

    PGConnection::ResultType res = conn->execParams("SELECT data_oid,launch FROM resultdata JOIN runresults ON runresults.runid=resultdata.runid WHERE resultdata.runid=$1 AND datatype=$2;",
        boost::tuples::make_tuple(IDr,dType));
    unpackSinglePGRow(res,boost::tuples::tie(data_oid,packets));

    switch(dType){
        case 1: data = new SurfaceFluenceMap(mesh); break;
        case 2: data = new VolumeFluenceMap(mesh);  break;
        default: throw string("Error in exportResultSet: invalid datatype");
    }

    string s = conn->loadLargeObject(data_oid);
    data->fromBinary(s);

    return data;
}



RunInfo getRunInfo(PGConnection* conn,unsigned IDr)
{
	RunInfo info;

	info.IDrun=IDr;

	PGConnection::ResultType res = conn->execParams("SELECT runs.caseid, runs.packets, cases.meshid, cases.materialsetid,cases.sourcegroupid "
			"FROM cases JOIN runs ON runs.caseid=cases.caseid "
			"WHERE runid=$1",
			boost::tuples::make_tuple(IDr));
	unpackSinglePGRow(res,boost::tuples::tie(info.IDcase,info.Npkt,info.IDmesh,info.IDmaterialset,info.IDsourcegroup));

	res = conn->execParams("SELECT datatype,data_oid,total,bytesize FROM resultdata WHERE runid=$1",boost::tuples::make_tuple(IDr));

	info.results.reserve(PQntuples(res.get()));

	for(int i=0;i<PQntuples(res.get());++i)
	{
		ResultInfo resinfo;
		unpackPGRow(res,boost::tuples::tie(resinfo.dtype,resinfo.oid,resinfo.sum,resinfo.Nbytes),i);
		info.results.push_back(resinfo);
	}

	return info;
}


/** Exports a data vector from the database
 *
 */

vector<double> exportResultVector(PGConnection* conn,unsigned IDr,unsigned dType)
{
	vector<double> v;

	cout << "INFO: Result data requested for run " << IDr << " data type " << dType << endl;
	PGConnection::ResultType res = conn->execParams("SELECT runs.caseid, runs.packets, cases.meshid, resultdata.data_oid,"
			"resultdata.total,resultdata.bytesize,meshes.npoints,meshes.nfaces,meshes.ntetras FROM runs "
			"JOIN cases ON runs.caseid=cases.caseid "
			"JOIN meshes ON meshes.meshid=cases.meshid "
			"JOIN resultdata ON resultdata.runid=runs.runid "
			"WHERE runs.runid=$1 AND resultdata.datatype=$2 "
			,boost::tuples::make_tuple(IDr,dType));

	unsigned long long Npkt;
	unsigned IDcase,IDmesh;
	Oid oid;
	double total;
	unsigned bytes,Np,Nf,Nt,N;

	unpackSinglePGRow(res,boost::tuples::tie(IDcase,Npkt,IDmesh,oid,total,bytes,Np,Nf,Nt));

	string b;

	switch(dType){
	case 1:
		cout << "Extracting surface fluence map" << endl;
		N = Nf+1;
		break;
	case 2:
		N = Nt+1;
		cout << "Extracting volume fluence map" << endl;
		break;
	default:
		cerr << "ERROR: Invalid data type requested" << endl;
		throw(std::string("invalid data type"));
	}

	b = conn->loadLargeObject(oid);

	v.resize(N,0.0);
	stringstream ss(b);

	struct __attribute__((packed)) { uint32_t id; double data; } idata;
	BOOST_STATIC_ASSERT(sizeof(idata)==12);

	cout << "  INFO: Read " << b.size() << " bytes from database (" << b.size()/12 << " 12B elements)" << endl;
	if (b.size() % 12 != 0)
		cerr << "  ERROR: size not a multiple of 12" << endl;

	while(!ss.eof())
	{
		ss.read((char*)&idata,12);
		v[idata.id] = idata.data;
	}

	return v;
}


vector<double> volumeEnergyToFluence(const vector<double>& energy,const TetraMesh& M,const vector<Material>& mats)
{
	vector<double> fluence(energy.size(),0.0);
	vector<double>::iterator fluence_it=fluence.begin();

	for(auto E : energy | boost::adaptors::indexed(0))
		*(fluence_it++) = E.index() ? E.value() / M.getTetraVolume(E.index()) / mats[M.getMaterial(E.index())].getMuA() : 0.0;
	return fluence;
}

vector<double> surfaceEnergyToFluence(const vector<double>& energy,const TetraMesh& M)
{
	vector<double> fluence(energy.size(),0.0);

	vector<double>::iterator fluence_it=fluence.begin();

	for(auto E : energy | boost::adaptors::indexed(0))
		*(fluence_it++) = E.index() ? E.value() / M.getFaceArea((unsigned)E.index()) : 0;
	return fluence;
}

