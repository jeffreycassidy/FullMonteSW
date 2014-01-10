#include <iostream>
#include "fm-postgres.hpp"
#include <iomanip>
#include <cstdlib>
#include <sstream>
#include <source.hpp>

#include "io_timos.hpp"

// need for ntohl / htonl
#include <netinet/in.h>

#include <newgeom.hpp>
#include <graph.hpp>

using namespace std;


map<string,string> connprops;
int DB_importMesh(PGConnection& dbconn,string fn,const TetraMesh& m);
int DB_importSourceGroup(PGConnection& dbconn,string fn,const vector<Source*>& sources,int meshid=1);
vector<Source*> readTIMOSSource(string fn);

int DB_exportSourceGroup(PGConnection& dbconn,vector<Source*>& sources,int sourcegroupid);
int DB_exportMaterialsGroup(PGConnection& dbconn,vector<Material>& materials,int materialgroupid);
int DB_importMaterialSet(PGConnection& dbconn,string fn,const vector<Source*>& sources,int meshid,string name="",string comment="");

class NullClass {
};

//template<>class PGBinaryRepresentation

//template<class T0=NullClass,class T1=NullClass,class T2=NullClass,class T3=NullClass,class T4=NullClass,class T5=NullClass,
//    class T6=NullClass,class T7=NullClass,class T8=NullClass,class T9=NullClass>

int main(int argc,char **argv)
{
    vector<Source*> sources;
    string fn,desc,name;

    connprops["host"]="localhost";
    connprops["port"]="5432";
    connprops["dbname"]="FMDB";
    connprops["user"]="postgres";

    PGConnection dbconn(connprops);

    dbconn.cleanLargeObjects();

    cout << "Files" << endl;

    for(unsigned i=1;i<(unsigned)argc;++i)
    {
        cout << argv[i] << endl;
    }
    cout << "==============================" << endl;

    for(unsigned i=1;i<(unsigned)argc;++i)
    {
        fn = argv[i];
        string sfx = getExtension(fn);

        if(sfx == "mesh")
        {
            cout << "Mesh file " << fn << endl;
            TetraMesh m(fn,TetraMesh::MatlabTP);
            int meshid = DB_importMesh(dbconn,argv[i],m);
            cout << "TetraMesh imported with meshid=" << meshid << endl;
        }
        else if (sfx == "source")
        {
            cout << "Source file " << fn << endl;
            vector<Source*> sources=readTIMOSSource(fn);
            try {
            int sourcegroupid = DB_importSourceGroup(dbconn,fn,sources);
            cout << "Source group imported with sourcegroupid=" << sourcegroupid << endl;
            }
            catch(PGConnection::PGConnectionException e)
            {
                cout << "Exception caught, message: " << e.msg << endl;
            }
        }
        else {
            cout << "Unrecognized file type for " << fn << endl;
        }
    }

    DB_exportSourceGroup(dbconn,sources,67);

    vector<Material> materials;

    for (unsigned i=0;i<=9;++i)
    {
        DB_exportMaterialsGroup(dbconn,materials,i);
    }

    writeTIMOSSource("test.source.out",sources);
    writeTIMOSMaterials("test.opt.out",materials);

    for(vector<Source*>::iterator it=sources.begin(); it != sources.end(); ++it)
        delete *it;

    // load the mesh
}

int DB_importMesh(PGConnection& dbconn,string fn,const TetraMesh& m)
{
    unsigned Np = m.getNp();
    unsigned Nt = m.getNt();
    unsigned Nf = m.getNf();

    Oid oid_points,oid_tetras;

    try {
        oid_points = dbconn.createLargeObject(m.pointsAsBinary());
        oid_tetras = dbconn.createLargeObject(m.tetrasAsBinary());
    } catch (class PGConnection::PGConnectionException e)
    {
        cout << "Exception - " << e.msg << endl;
    }

//    cout << "Saved as large object " << oid_points << endl;

    string query("INSERT INTO meshes(npoints,ntetras,nfaces,sourcefn,description,name,pdata_oid,tdata_oid) "
        "VALUES ($1::int,$2::int,$3::int,$4,$5,$6,$7::Oid,$8::Oid) RETURNING meshid");

    int paramFormats[8] = { 1,1,1,0,0,0,1,1 };  // 1 for binary, 0 for text
    int paramLengths[8] = { 4,4,4,0,0,0,4,4 };

    Oid net_oid_points=htonl(oid_points),net_oid_tetras=htonl(oid_tetras);
    uint32_t net_Np=htonl(Np),net_Nt=htonl(Nt),net_Nf=htonl(Nf);

    const char * const params[] = {
        (const char *)&net_Np,
        (const char *)&net_Nt,
        (const char *)&net_Nf,
        fn.c_str(),
        "newdescription",
        "newname",
        (const char *)&net_oid_points,
        (const char *)&net_oid_tetras
    };

    SmartResult prepres = dbconn.prepare("",query.c_str(),8,NULL);
    SmartResult res = dbconn.execPrepared("",8,params,paramLengths,paramFormats,PGConnection::ResultsBinary);

    return *((int*)PQgetvalue(res,0,0));
}

int DB_importSourceGroup(PGConnection& dbconn,string fn,const vector<Source*>& sources,int meshid)
{
    string query("INSERT INTO sourcegroups(sourcefn,meshid,comment) "
        "VALUES ($1,$2::int,$3) RETURNING sourcegroupid");

    int paramFormats[3] = { 0,1,0 };
    int paramLengths[3] = { 0,4,0 };

    uint32_t net_meshid=htonl(meshid);

    const char * const params[] = {
        fn.c_str(),
        (const char*)&net_meshid,
        "source group description"
    };

    SmartResult prepres = dbconn.prepare("",query.c_str(),3,NULL);
    SmartResult res = dbconn.execPrepared("",3,params,paramLengths,paramFormats,PGConnection::ResultsBinary);

    uint32_t net_sourcegroupid = *(int*)PQgetvalue(res,0,0);
    int sourcegroupid= ntohl(net_sourcegroupid);

    cout << "Added source group ID=" << sourcegroupid << endl;

    uint32_t IDt,IDf;
    char w_buf[20];

    const char* const pvals2[3] = {
        (const char*)&net_sourcegroupid,
        (const char*)w_buf,
        (const char*)&IDt
    };

    int plengths2[3] = { 4,0,4 };
    int pfmts2[3] = { 1,0,1 };


    for(vector<Source*>::const_iterator it=sources.begin(); it != sources.end(); ++it)
    {
        sprintf(w_buf,"%g",(*it)->getPower());
        SmartResult res=NULL;
        if (VolumeSource* vol = dynamic_cast<VolumeSource*>(*it))
        {
            IDt=htonl(vol->getIDt());
            res = dbconn.execParams("SELECT insert_source_vol($1::int,$2::double precision,$3::int)",3,NULL,
                pvals2,plengths2,pfmts2,PGConnection::ResultsBinary);
        }
        else if (IsotropicPointSource* ips = dynamic_cast<IsotropicPointSource*>(*it))
        {
            char pos_x_buf[20],pos_y_buf[20],pos_z_buf[20];
            sprintf(pos_x_buf,"%g",ips->getOrigin()[0]);
            sprintf(pos_y_buf,"%g",ips->getOrigin()[1]);
            sprintf(pos_z_buf,"%g",ips->getOrigin()[2]);

            const char* const ips_paramValues[5] = 
            {
                (const char*)&net_sourcegroupid,
                (const char*)w_buf,
                (const char*)pos_x_buf,
                (const char*)pos_y_buf,
                (const char*)pos_z_buf
            };
            int ips_paramLengths[5] = { 4,0,0,0,0 };
            int ips_paramFormats[5] = { 1,0,0,0,0 };
            res = dbconn.execParams("SELECT insert_source_ips($1::int,$2::double precision,$3::double precision,$4::double precision,$5::double precision);",
                5,NULL,ips_paramValues,ips_paramLengths,ips_paramFormats,PGConnection::ResultsBinary);
        }
        else if (PencilBeamSource* pbs = dynamic_cast<PencilBeamSource*>(*it))
        {
            char posbuf[3][20],dirbuf[3][20];
            for(unsigned i=0;i<3;++i){
                sprintf(posbuf[i],"%g",pbs->getOrigin()[i]);
                sprintf(dirbuf[i],"%g",pbs->getDirection()[i]);
            }
            IDt = htonl(pbs->getIDt());

            const char* const pbs_paramValues[9] = 
            {
                (const char*)&net_sourcegroupid,
                (const char*)w_buf,
                (const char*)&IDt,
                (const char*)posbuf[0],
                (const char*)posbuf[1],
                (const char*)posbuf[2],
                (const char*)dirbuf[0],
                (const char*)dirbuf[1],
                (const char*)dirbuf[2]
            };
            int pbs_paramLengths[9] = { 4,0,4,0,0,0,0,0,0 };
            int pbs_paramFormats[9] = { 1,0,1,0,0,0,0,0,0 };
            res = dbconn.execParams("SELECT insert_source_pbs($1::int,$2::double precision,$3::int,$4::double precision,$5::double precision,$6::double precision,$7::double precision,$8::double precision,$9::double precision);",
                9,NULL,pbs_paramValues,pbs_paramLengths,pbs_paramFormats,PGConnection::ResultsBinary);
        }
        else if (FaceSource* face = dynamic_cast<FaceSource*>(*it)){
            IDf = htonl(0);
            IDt = htonl(face->getIDt());
            FaceByPointID f = face->getIDps();
            char f_buf[40];
            sprintf(f_buf,"{%d,%d,%d}",f[0],f[1],f[2]);
            const char* const face_paramValues[9] = 
            {
                (const char*)&net_sourcegroupid,
                (const char*)w_buf,
                (const char*)&IDf,
                (const char*)&IDt,
                (const char*)f_buf
            };
            int face_paramLengths[5] = { 4,0,4,4,0 };
            int face_paramFormats[5] = { 1,0,1,1,0};
            res = dbconn.execParams("SELECT insert_source_tri($1::int,$2::double precision,$3::int,$4::int,$5::int[3]);",
                5,NULL,face_paramValues,face_paramLengths,face_paramFormats,PGConnection::ResultsBinary);
        }
        else
            cerr << "Unrecognized type" << endl;
//        if(res)
//            int sourceID = ntohl(*(int*)PQgetvalue(res,0,0));
    }

    return sourcegroupid;
}

int DB_importMaterialSet(PGConnection& dbconn,string fn,const vector<Source*>& sources,int meshid,string name,string comment)
{
    return 0;
/*    // add the material set 
    string query("INSERT INTO materialsets(sourcefn,name,comment) VALUES ($1,$2,$3) RETURNING materialsetid;");

    int paramFormats[3] = { 0,0,0 };
    int paramLengths[3] = { 0,0,0 };

    const char * const params[] = {
        fn.c_str(),
        "material set name",
        "material set comment"
    };

    PGresult* res = dbconn.execParams(query.c_str(),3,params,paramLengths,paramFormats,PGConnection::ResultsBinary);

    uint32_t net_materialsetid = *(uint32_t*)PQgetvalue(res,0,0);


    // insert materials one-by-one
    string matquery("INSERT INTO materials(mu_a,mu_s,g,n,matchedboundary,materialsetid,name,comment) VALUES "\
        "($1::double precision,$2::double precision,$3::double precision,$4::double precision,$5::boolean,$6::int,$7,$8) "\
        "RETURNING materialid;");

    uint64_t net_mu_a,net_mu_s,net_g,net_n,net_matchedboundary,net_materialsetid,w_buf;

    const char* const matparams[8] = {
        (const char*)&net_mu_a,
        (const char*)&net_mu_s,
        (const char*)&net_g,
        (const char*)&net_n,
        (const char*)&net_matchedboundary,
        (const char*)&net_materialsetid,
        (const char*)name.c_str(),
        (const char*)comment.c_str(),
        "materials name",
        "materials comment"
    };

    int matValues[8]  = { 8,8,8,8,4,4,0,0 };
    int matFormats[8] = { 1,1,1,1,1,1,0,0 };


    for(vector<Source*>::const_iterator it=sources.begin(); it != sources.end(); ++it)
    {

        if (VolumeSource* vol = dynamic_cast<VolumeSource*>(*it))
        {
            IDt=htonl(vol->getIDt());
            res = dbconn.execParams("SELECT insert_source_vol($1::int,$2::double precision,$3::int)",3,NULL,
                pvals2,plengths2,pfmts2,PGConnection::ResultsBinary);
        }
        else if (IsotropicPointSource* ips = dynamic_cast<IsotropicPointSource*>(*it))
        {
            char pos_x_buf[20],pos_y_buf[20],pos_z_buf[20];
            sprintf(pos_x_buf,"%g",ips->getOrigin()[0]);
            sprintf(pos_y_buf,"%g",ips->getOrigin()[1]);
            sprintf(pos_z_buf,"%g",ips->getOrigin()[2]);

            const char* const ips_paramValues[5] = 
            {
                (const char*)&net_sourcegroupid,
                (const char*)w_buf,
                (const char*)pos_x_buf,
                (const char*)pos_y_buf,
                (const char*)pos_z_buf
            };
            int ips_paramLengths[5] = { 4,0,0,0,0 };
            int ips_paramFormats[5] = { 1,0,0,0,0 };
            res = dbconn.execParams("SELECT insert_source_ips($1::int,$2::double precision,$3::double precision,$4::double precision,$5::double precision);",
                5,NULL,ips_paramValues,ips_paramLengths,ips_paramFormats,PGConnection::ResultsBinary);
        }

        if(res)
            int sourceID = ntohl(*(int*)PQgetvalue(res,0,0));
    }

    return sourcegroupid;*/
}

int DB_exportSourceGroup(PGConnection& dbconn,vector<Source*>& sources,int sourcegroupid)
{
    sources.clear();

    string query("SELECT tetraid,default_w FROM sources JOIN sources_vol ON sources_vol.sourceid=sources.sourceid "\
        "WHERE sourcetype='V' AND sourcegroupid=$1::int");

    int paramFormats[3] = { 1 };
    int paramLengths[3] = { 4 };

    uint32_t net_sourcegroupid=htonl(sourcegroupid);

    const char * const params[] = {
        (const char *)& net_sourcegroupid
    };

    SmartResult prepres = dbconn.prepare("",query.c_str(),1,NULL);
    SmartResult res = dbconn.execPrepared("",1,params,paramLengths,paramFormats,PGConnection::ResultsBinary);

    unsigned Ns=PQntuples(res);

    unsigned IDt;

    uint64_t blah;
    uint64_t msb,lsb;

    union float8_t {
        double d;
        uint64_t i;
    };

    sources.resize(Ns);
    vector<Source*>::iterator s=sources.begin();

    for(unsigned i=0;i<Ns;++i)
    {
        IDt = ntohl(*(int*)PQgetvalue(res,i,0));
        blah = *(uint64_t*)PQgetvalue(res,i,1);

        union float8_t default_w;
        msb = ntohl(blah >> 32);
        lsb = ntohl(blah & 0xFFFFFFFF);
        default_w.i = ((uint64_t)(htonl(blah>>32))) | ((uint64_t)(htonl(blah&0xFFFFFFFF)) << 32);

        *(s++) = new VolumeSource(IDt,default_w.d);
    }
    return Ns;
}

union float8_t {
    double d;
    uint64_t i;
};

double convDouble(const char* s)
{
    uint64_t msb,lsb;
    union float8_t val;

    msb = ntohl(*(uint64_t*)s >> 32);
    lsb = ntohl(*(uint64_t*)s & 0xFFFFFFFF);

    val.i = (lsb << 32) | msb;

    return val.d;
}

int DB_exportMaterialsGroup(PGConnection& dbconn,vector<Material>& materials,int materialgroupid)
{
    materials.clear();

    string query("SELECT mu_a,mu_s,g,n,matchedboundary,materialset_map.materialindex FROM materials JOIN materialset_map ON " \
        "materialset_map.materialid=materials.materialid WHERE materialset_map.materialsetid=$1::int ORDER BY materialset_map.materialindex");

    int paramFormats[1] = { 1 };
    int paramLengths[1] = { 4 };

    uint32_t net_materialgroupid=htonl(materialgroupid);

    const char * const params[] = {
        (const char *)& net_materialgroupid
    };

    SmartResult prepres = dbconn.prepare("",query.c_str(),1,NULL);
    SmartResult res = dbconn.execPrepared("",1,params,paramLengths,paramFormats,PGConnection::ResultsBinary);

    unsigned Nm=PQntuples(res);
    double mu_s,mu_a,g,n;

    materials.resize(Nm);
    vector<Material>::iterator m=materials.begin();

    int idx;
    bool matchedboundary;

    for(unsigned i=0;i<Nm;++i)
    {
        mu_a= convDouble(PQgetvalue(res,i,0));
        mu_s= convDouble(PQgetvalue(res,i,1));
        g   = convDouble(PQgetvalue(res,i,2));
        n   = convDouble(PQgetvalue(res,i,3));
        matchedboundary = *(uint32_t*)PQgetvalue(res,i,4);
        idx = ntohl(*(uint32_t*)PQgetvalue(res,i,5));

        if (idx != (int)i)
            cerr << "Material out of sequence: expecting " << i << " found " << idx << endl;

        *(m++) = Material(mu_a,mu_s,g,n,false);
        cout << mu_a << ' ' << mu_s << ' ' << g << ' ' << n << endl;
    }
    return Nm;
}
