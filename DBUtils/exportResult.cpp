#include <iostream>

#include "fluencemap.hpp"

#include "fm-postgres/fm-postgres.hpp"

extern char** environ;

using namespace std;

map<unsigned,unsigned long long> loadVolumeHitMap(PGConnection& dbconn,unsigned IDrun);

int main(int argc,char **argv)
{
    // set up command line vars
    boost::program_options::options_description cmdline;
    boost::program_options::variables_map vm;

    unsigned dtype=1;
    unsigned runid=0;

    string ofn("op.txt");

    cmdline.add_options()
        ("run,R",   boost::program_options::value<unsigned>(&runid),"Specify run ID to export")
        ("dtype,d", boost::program_options::value<unsigned>(&dtype),"Data type to export")
        ("outfile,o",boost::program_options::value<string>(&ofn),"Output file name")
        ;

    cmdline.add(globalopts::db::dbopts);

    // parse command line and environment variables
    boost::program_options::store(boost::program_options::command_line_parser(argc,argv).options(cmdline).run(),vm);
    boost::program_options::store(parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
    boost::program_options::notify(vm);

    if(!vm.count("dtype"))
        cout << "Loading default data type 1" << endl;
    if(!vm.count("run"))
    {
        cerr << "Error: no dataset specified for export" << endl;
        return 1;
    }

    // connect and query for comparison results
    boost::shared_ptr<PGConnection>  conn;
    try {
        conn=PGConnect();

        /*boost::shared_ptr<PGresult> res = conn->execParams("SELECT data_oid,datatype,packets,cases.caseid,cases.meshid,meshes.ntetras,meshes.nfaces "\
            "FROM runs_data AS rd LEFT JOIN cases ON rd.caseid=cases.caseid "\
            "LEFT JOIN meshes ON meshes.meshid = cases.meshid WHERE datatype=$2 AND runid=$1;",
            boost::tuples::make_tuple(runid,dtype));*/



        unsigned IDm,IDc;
    
        Oid data_oid;
        unsigned long long Nph;
        unsigned Nf,Nt;
        unsigned datatype_ret;

            unpackSinglePGRow(res,boost::tuples::tie(data_oid,datatype_ret,Nph,IDc,IDm,Nt,Nf));
            TetraMesh* mesh = exportMesh(*(conn.get()),IDc);

            vector<Material> mat;
            exportMaterials(*(conn.get()),IDc,mat);

            ofstream os(ofn.c_str());
            os << "# Exported by exportResult" << endl;
            os << "#   Run ID:       " << runid << endl;
            os << "#   Packet count: " << Nph << endl;
            os << "#   Data type:    " << dtype << endl;
            os << "#   Mesh ID:      " << IDm << endl;
            os << "#   Case ID:      " << IDc << endl;

            if (dtype == 1)
            {
                SurfaceFluenceMap ref(*mesh,conn->loadLargeObject(data_oid));
                os << Nf << endl;
    
                for(SurfaceFluenceMap::const_energy_iterator it=ref.energyBegin(); it != ref.energyEnd(); ++it)
                    os << it->first << " " << it->second << endl;
    //                os << "0 0 " << mesh->getTetraVolume(it->first) << ' ' << it->second << endl;
            }
            else if (dtype == 2)
            {
                os << Nt << endl;
                VolumeFluenceMap ref(*mesh,conn->loadLargeObject(data_oid));
                for(VolumeFluenceMap::const_energy_iterator it=ref.energyBegin(); it != ref.energyEnd(); ++it)
                    os << it->first << " " << it->second*mat[mesh->getMaterial(it->first)].getMuA() << endl;
    //                os << "0 0 " << mesh->getTetraVolume(it->first) << ' ' << it->second << endl;
            }
            else if (dtype == 4)
            {
                map<unsigned,unsigned long long> m;
                
                m = loadVolumeHitMap(*conn,runid);
                for(map<unsigned,unsigned long long>::const_iterator it=m.begin(); it != m.end(); ++it)
                    os << it->first << " " << it->second << endl;
            }
        }
    catch (PGConnection::PGConnectionException &e){
        cerr << e.msg << endl;
        exit(-1);
    }
    catch (string s){
        cerr << "Caught an exception with string: " << s << endl;
    }

}



map<unsigned,unsigned long long> loadVolumeHitMap(PGConnection& dbconn,unsigned IDrun)
{
    unsigned dtype=4;
    stringstream qrystr;
    qrystr << "SELECT data_oid,total,bytesize FROM resultdata WHERE datatype=$2 AND runid=$1;";

    cout << qrystr.str() << endl;

    PGConnection::ResultType res = dbconn.execParams(qrystr.str().c_str(),boost::tuples::make_tuple(IDrun,dtype));

    Oid data_oid;
    double total;
    unsigned bytesize;

    unpackSinglePGRow(res,boost::tuples::tie(data_oid,total,bytesize));

/*    cout << "First record (meshid " << meshid << ") has " << Np << " points, " << Nt << " tetras, and " << Nf << " faces" << endl;
    cout << "  pdata_oid=" << pdata_oid << ", tdata_oid=" << tdata_oid << ", fdata_oid=" << fdata_oid << endl;
    cout << "  name=" << name << " description=" << desc << endl;*/

    string b = dbconn.loadLargeObject(data_oid);

//    cout << "Read " << bpoints.getSize() << " bytes of points (" << bpoints.getSize()/3/sizeof(double) << " points)" << endl;
//    cout << "Read " << btetras.getSize() << " bytes of tetras (" << btetras.getSize()/5/sizeof(unsigned) << " tetras)" << endl;

/*    if ((unsigned)bpoints.getSize()/3/sizeof(double) != (unsigned)Np || (unsigned)btetras.getSize()/5/sizeof(unsigned) != (unsigned)Nt)
    {
        cerr << "Error: size mismatch!" << endl;
        return NULL;
    }*/

    map<unsigned,unsigned long long> m;


/*    m.fromBinary(b);*/

    unsigned long long sum=0;

//    map<unsigned,unsigned long long> m;

//    cout << "Exported " << b.getSize() << " bytes, expecting " << bytesize << endl;

    for(const uint8_t* p=b.data(); p<b.data()+b.size(); p+=12)
    {
        unsigned id = *((uint32_t*)p);
        unsigned long long count = *((uint64_t*)(p+4));

        m.insert(make_pair(id,count));
        sum += count;
    }

    cout << "Read " << m.size() << " values, total event count: " << sum << endl;

    return m;
}
