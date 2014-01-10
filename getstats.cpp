#include "graph.hpp"
#include "fluencemap.hpp"
#include "fmdb.hpp"
#include "source.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <fstream>

namespace po=boost::program_options;

using namespace std;

int main(int argc,char **argv)
{
    unsigned N,dtype=atoi(argv[1]),IDm,IDf=atoi(argv[2]);
    bool reportEnergy=true;     // report total energy or per-volume/per-area?
    double wmin=0;
    unsigned seed;
    Oid oid_data;

    po::variables_map vm;
    po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
    po::notify(vm);

    boost::shared_ptr<PGConnection> dbconn;
    PGConnection::ResultType res;
    TetraMesh* m=NULL;

    try {
        dbconn=PGConnect();

        res = dbconn->execParams("SELECT meshid, data_oid, wmin, seed, caseid FROM runs_fluence " \
            "WHERE flightid=$1 AND datatype=$2 "\
            "ORDER BY meshid,caseid,wmin,seed",
            boost::tuples::make_tuple(IDf,dtype));

    }
    catch(PGConnection::PGConnectionException& e)
    {
        cerr << "Database exception: " << e.msg << endl;
        exit(-1);
    }
    catch(string& s)
    {
        cerr << "Exception string: " << s << endl;
        exit(-1);
    }

    vector<unsigned> setLengths;
    unsigned IDm_last=-1,IDc_last=-1,IDc;
    double wmin_last=-1;

    for(int r=0; r<PQntuples(res.get()); ++r)
    {
        unpackPGRow(res,boost::tuples::tie(IDm,oid_data,wmin,seed,IDc),r);
        if (IDm==IDm_last && wmin==wmin_last && IDc_last==IDc)
            ++setLengths.back();
        else
        {
            cout << "**** Result set " << setLengths.size()+1 << endl;
            setLengths.push_back(1);
        }
        cout << setw(5) << IDm << " " << setw(6) << oid_data << " " << setw(8) << wmin << " " << setw(4) << seed << endl;
        IDm_last=IDm;
        IDc_last=IDc;
        wmin_last=wmin;
    }

    IDm_last=-1;
    unsigned r=0;
    unsigned IDset=0;

    // Loop over all result sets
    for(vector<unsigned>::const_iterator it=setLengths.begin(); it != setLengths.end(); ++it,++IDset)
    {
        cout << "Result set " << IDset << " with " << *it << " elements" << endl;
        unpackPGRow(res,boost::tuples::tie(IDm,oid_data,wmin,seed,IDc),r);

        // Load mesh if necessary
        if (IDm_last != IDm)
        {
            delete m;
            m = exportMesh(*(dbconn.get()),IDm);

            switch(dtype){
                case 1: N=m->getNf();    break;
                case 2: N=m->getNt();             break;
                default: cerr << "Unexpected data type" << endl; exit(-1);
            }
            IDm_last=IDm;
        }

        unsigned Nsets=*it;

        // create storage
        double **ptr = new double*[Nsets];
        double *data = new double[Nsets*N];

        for(unsigned i=0;i<Nsets*N;++i)
            data[i]=0;

        for(unsigned i=0;i<Nsets;++i,++r)
        {
            unpackPGRow(res,boost::tuples::tie(IDm,oid_data,wmin,seed,IDc),r);
            ptr[i]=data + N*i;
    
            if (dtype == 1)
            {
                SurfaceFluenceMap F(m);
                F.fromBinary(dbconn.get()->loadLargeObject(oid_data));
                if (reportEnergy){
                    double sum=0.0;
                    for(SurfaceFluenceMap::const_energy_iterator it=F.energyBegin(); it != F.energyEnd(); ++it)
                    {
                        sum += (*it).second;
                        ptr[i][(*it).first-1]=(*it).second;
                    }
                    cout << "Total energy: " << sum << endl;
                }
                else {
                    for(SurfaceFluenceMap::const_iterator it=F.begin(); it != F.end(); ++it)
                        ptr[i][(*it).first-1]=(*it).second;
                }
            }
            else if (dtype == 2)
            {
                VolumeFluenceMap F(m);
                F.fromBinary(dbconn.get()->loadLargeObject(oid_data));
                if (reportEnergy){
                    for(VolumeFluenceMap::const_energy_iterator it=F.energyBegin(); it != F.energyEnd(); ++it)
                        ptr[i][(*it).first-1]=it->second;
                }
                else {
                    for(VolumeFluenceMap::const_iterator it=F.begin(); it != F.end(); ++it)
                        ptr[i][(*it).first-1]=it->second;
                }
            }
            else {
                cerr << "Unexpected data type " << dtype << endl;
                exit(-1);
            }
        }

        stringstream ss;

        ss << "output." << IDset << ".txt";

        ofstream os(ss.str().c_str(),ios_base::out);
        os << N << ' ' << Nsets << endl;
        for(unsigned i=0;i<N;++i)
        {
            for(unsigned j=0;j<Nsets;++j)
                os << ptr[j][i] << ' ';
            os << endl;
        }
        os.close();

        delete[] ptr;
        delete[] data;
    }
}
