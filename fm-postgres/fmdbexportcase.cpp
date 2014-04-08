#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <source.hpp>
#include "fm-postgres.hpp"

#include "io_timos.hpp"
#include "source.hpp"
#include "graph.hpp"

using namespace std;

int exportSources(PGConnection& dbconn,unsigned IDsourcegroup,vector<Source*>& sources,long long Npacket)
{
    sources.clear();

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
        IsotropicPointSource* s = new IsotropicPointSource(p,default_w);
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
        VolumeSource* s = new VolumeSource(IDt,default_w);
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
        FaceSource* s = new FaceSource(IDps,default_w);
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
        PencilBeamSource* s = new PencilBeamSource(p,UnitVector<3,double>(dir),default_w,IDt);
        sources.push_back(s);
    }

    return sources.size();
}

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

    Blob bpoints = dbconn.loadLargeObject(pdata_oid);
    Blob btetras = dbconn.loadLargeObject(tdata_oid);

    cout << "Read " << bpoints.getSize() << " bytes of points (" << bpoints.getSize()/3/sizeof(double) << " points)" << endl;
    cout << "Read " << btetras.getSize() << " bytes of tetras (" << btetras.getSize()/5/sizeof(unsigned) << " tetras)" << endl;

    if ((unsigned)bpoints.getSize()/3/sizeof(double) != (unsigned)Np || (unsigned)btetras.getSize()/5/sizeof(unsigned) != (unsigned)Nt)
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

FluenceMapBase* exportResultSet(PGConnection* conn,unsigned IDr,unsigned dType,const TetraMesh* mesh=NULL)
{
    unsigned long long packets;
    FluenceMapBase* data;

    Oid data_oid;

    PGConnection::ResultType res = conn->execParams("SELECT data_oid,launch FROM resultdata JOIN runresults ON runresults.runid=resultdata.runid WHERE resultdata.runid=$1 AND datatype=$2;",
        boost::tuples::make_tuple(IDr,dType));
    unpackSinglePGRow(res,boost::tuples::tie(data_oid,packets));

//    cout << "Run " << globalopts::runs[0] << ": " << packets_a << " launched" << endl;

    switch(dType){
        case 1: data = new SurfaceFluenceMap(mesh); break;
        case 2: data = new VolumeFluenceMap(mesh);  break;
        default: throw string("Error in exportResultSet: invalid datatype");
    }

    Blob b = conn->loadLargeObject(data_oid);
    data->fromBinary(b,packets);

    return data;
}
