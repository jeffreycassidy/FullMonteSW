#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

#include "graph.hpp"
#include "io_timos.hpp"
#include <signal.h>
/*#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>*/

#include "fmdb.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include <map>

namespace po=boost::program_options;
/*
namespace globalopts {
    double Npkt;               // number of packets
    long Nk=0;                 // number of packets as long int
    unsigned Nthread=1;
    unsigned randseed=1;
    string logFN("log.out");   // log filename
    string outpath(".");       // output path, defaults to working dir
    bool dbwrite=true;
    double wmin=0.00001;
    double prwin=0.1;
}
*/
using namespace std;

int main(int argc,char **argv)
{
    unsigned IDc=0;
    string fn_mesh;
    vector<Source*> sources;

    // define command-line options
    po::options_description cmdline("Command-line options");
    po::positional_options_description pos;
    pos.add("input",1).add("materials",1).add("sourcefile",1);

    cmdline.add_options()
        ("help,h","Display option help")
        ("input,i",po::value<string>(&fn_mesh),"Input file")
//        ("mesh,m",po::value<unsigned>(&IDm),"Mesh to export from database")
        ("case,c",po::value<unsigned>(&IDc),"Cases to run from database")
//        ("suite,s",po::value<vector<unsigned> >(&suites),"Suites to run from database")
        ;

    cmdline.add(globalopts::db::dbopts);

    // parse options (if an option is already set, subsequent store() calls will not overwrite
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc,argv).options(cmdline).run(),vm);
        po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
        po::notify(vm);
    }
    catch (po::error& e){
        cerr << "Caught an exception in options processing" << endl;
        cout << cmdline << endl;
        return -1;
    }

    boost::shared_ptr<PGConnection> dbconn;

        try {
            dbconn = PGConnect();
        }
        catch(PGConnection::PGConnectionException& e)
        {
            cerr << "Failed to connect with error: " << e.msg << endl;
        }

    // apply options
    if (vm.count("help"))
    {
        cout << cmdline << endl;
        return -1;
    }
	cout << endl;

    TetraMesh *m = exportMesh(*(dbconn.get()),IDc);

    cout << "Loaded mesh from database" << endl;

    cout << "Tetras: " << m->getNt() << endl;
    cout << "Faces:  " << m->getNf() << endl;
    cout << "Surface faces: " << m->getNf_boundary() << endl;

    return 0;
/*
    if (vm.count("input"))
        TetraMesh M(fn_mesh,TetraMesh::MatlabTP);

    return 0;// run based on a case ID*/

}
/*
void runCaseByID(PGConnection* dbconn,unsigned IDflight,unsigned IDcase,unsigned long long Nk)
{
    TetraMesh m;
    Oid pdata_oid,tdata_oid;

    PGConnection::ResultType res = dbconn->execParams("SELECT meshes.pdata_oid,meshes.tdata_oid,cases.sourcegroupid,cases.materialsetid FROM cases " \
        "JOIN meshes ON meshes.meshid=cases.meshid " \
        "WHERE caseid=$1",
        boost::tuples::make_tuple(IDcase));

    unpackSinglePGRow(res,boost::tuples::tie(pdata_oid,tdata_oid,IDsourcegroup,IDmaterials));

    m.fromBinary(dbconn->loadLargeObject(pdata_oid),dbconn->loadLargeObject(tdata_oid));
}
*/
