#include <iostream>
#include "fluencemap.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "MatFile.hpp"

#include "blob.hpp"

#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/assign.hpp>

#include <map>

using namespace std;

extern char** environ;

/** Extracts a sparse result set as an (ID,value) map
 *
 */

template<class K,class V>vector<K> keys(const map<K,V>& m)
{
	vector<K> v(m.size());
	typename vector<K>::iterator it=v.begin();
	for(const pair<K,V>& p : m)
		*(it++)=p.first;

	return v;
}

template<class K,class V>vector<V> values(const map<K,V>& m)
{
	vector<V> v(m.size());
	typename vector<V>::iterator it=v.begin();
	for(const pair<K,V>& p : m)
		*(it++)=p.second;

	return v;
}

/** Takes a map representing a sparse vector and returns a dense vector of N elements
 *
 */

template<class K,class V>vector<V> desparsify(const map<K,V>& m,K N)
{
	vector<V> v(N+1);

	for(const pair<K,V>& p : m)
		v[p.first] = p.second;

	return v;
}

map<unsigned long,double> extractSparse(PGConnection* conn,Oid data_oid)
{
	map<unsigned long,double> m;
	double sum=0.0;

	Blob b = conn->loadLargeObject(data_oid);
	// cout << "Exported " << b.getSize() << " bytes, expecting " << bytesize << endl;

	for(const uint8_t* p=b.getPtr(); p<b.getEndPtr(); p+=12)
	{
		unsigned id = *((uint32_t*)p);
		double count = *((double*)(p+4));

		m.insert(make_pair(id,count));
		sum += count;
	}

	cout << "Read " << m.size() << " values with total value " << sum << endl;

	return m;
}




/** Extracts a dense result set as a vector indexed by ID
 *
 */

vector<double> extractDense(PGConnection* conn,Oid data_oid,unsigned N)
{
	map<unsigned long,double> m = extractSparse(conn,data_oid);

	vector<double> v(N,0.0);

	for(pair<unsigned long,double> p : m)
		v[p.first]=p.second;

	return v;
}

typedef struct {
	Oid data_oid;
	unsigned dtype;
	unsigned N;
} ResultInfo;


void listResults(PGConnection* conn,unsigned IDrun)
{
	unsigned long long Npkt;
	unsigned IDc,IDm,Nt,Nf;

	// get run information
	boost::shared_ptr<PGresult> res = conn->execParams("SELECT packets,caseid,meshes.meshid,ntetras,nfaces "\
				"FROM flights_runs JOIN meshes ON meshes.meshid = flights_runs.meshid WHERE runid=$1;",
				boost::tuples::make_tuple(IDrun));

	cout << "Information for run " << IDrun << endl << "================================" << endl;

	unpackSinglePGRow(res,boost::tuples::tie(Npkt,IDc,IDm,Nt,Nf));

	cout << "  Packets:  " << Npkt << endl;
	cout << "  Mesh ID:  " << IDm << endl;
	cout << "  Case ID:  " << IDc << endl;
	cout << "  Tetras:   " << Nt  << endl;
	cout << "  Faces:    " << Nf  << endl;
	cout << endl;


	// get dataset information linked to this run
	res = conn->execParams("SELECT data_oid,datatype "\
			"FROM runs_data AS rd LEFT JOIN cases ON rd.caseid=cases.caseid "\
			"LEFT JOIN meshes ON meshes.meshid = cases.meshid WHERE runid=$1;",
			boost::tuples::make_tuple(IDrun));

	vector<ResultInfo> infos;

	for(unsigned i=0;i<PQntuples(res.get());++i)
	{
		ResultInfo info;

		unpackPGRow(res,boost::tuples::tie(info.data_oid,info.dtype),i);
		cout << setw(6) << info.data_oid << ": type " <<
				info.dtype << " (";

		switch(info.dtype)
		{
		case 1:		// surface fluence map
			cout << "surface fluence map";
			break;

		case 2:		// volume fluence map
			cout << "volume fluence map";
			break;

		case 3:		// surface hit map
			cout << "surface hit map";
			break;

		case 4:		// volume hit map
			cout << "volume hit map";
			break;


		case 5:
			cout << "????5";
			break;
		case 6:
			cout << "????6";
			break;
		case 7:
			cout << "????7";
			break;
		case 8:
			cout << "????8";
			break;

		default:
			throw string("Invalid data type");
		}

		cout << ')' << endl;

		infos.push_back(info);
	}

	cout << "Exporting data Oid " << infos.at(0).data_oid << " to output.mat" << endl;

	map<unsigned long,double> m = extractSparse(conn,infos.at(0).data_oid);

	{
		ofstream os("output.mat");

		MatFile::Header hdr;

		os.write((const char*)&hdr,sizeof(MatFile::Header));

		vector<double> in(m.size(),0.0);
		vector<unsigned long> k = keys(m);

		copy(k.begin(),k.end(),in.begin());

		writeElement(os,"sparse_indices",in);
		writeElement(os,"sparse_values",values(m));

		writeElement(os,"dense",desparsify(m,(unsigned long)Nf));
	}
}


int main(int argc,char **argv)
{
    // set up command line vars
    boost::program_options::options_description cmdline;
    boost::program_options::variables_map vm;

    unsigned dtype=1;
    unsigned IDrun=0;

    string ofn("op.txt");

    cmdline.add_options()
        ("run,R",   boost::program_options::value<unsigned>(&IDrun),"Specify run ID to export")
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


    // TODO: Make this work with more than one run
    if(vm.count("run") != 1)
    {
        cerr << "Error: must specify exactly one run to export" << endl;
        return 1;
    }

    // connect
    boost::shared_ptr<PGConnection>  conn;
    try {
        conn=PGConnect();
    }
    catch (PGConnection::PGConnectionException &e){
        cerr << e.msg << endl;
        exit(-1);
    }
    catch (string s){
        cerr << "Caught an exception with string: " << s << endl;
    }

    // query for results
    try {
    	listResults(conn.get(),IDrun);
    }
    catch (PGConnection::PGConnectionException &e){
        cerr << e.msg << endl;
        exit(-1);
    }
    catch (string s){
        cerr << "Caught an exception with string: " << s << endl;
    }
}
