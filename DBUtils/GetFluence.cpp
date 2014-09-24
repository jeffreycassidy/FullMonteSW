#include "graph.hpp"
#include "fmdb.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include "../Sequence.hpp"
#include <vector>

#include <boost/range/adaptor/indexed.hpp>

namespace po=boost::program_options;

using namespace std;

void writeASCII(string fn,const vector<double>& V)
{
	ofstream os(fn.c_str());
	for(double v : V)
		os << v << endl;
}

vector<double> volumeEnergyToFluence(const vector<double>& energy,const TetraMesh& M,const vector<Material>& mats);
vector<double> surfaceEnergyToFluence(const vector<double>& energy,const TetraMesh& M);

template<class T,class U>bool CompareSecondDesc(const pair<T,U>& a,const pair<T,U>& b){ return a.second > b.second; }

//template<class Iterator>void writeSortedFluence_ASCII(string fn,Iterator begin,Iterator end,unsigned N=0);

int main(int argc,char **argv)
{
    po::variables_map vm;

    po::options_description cmdline;

    unsigned IDflight=-1,IDcase=-1;
    Sequence<unsigned> runs;
    string pfx("out.");

    cmdline.add_options()
    		("surface,s","Extract surface")
    		("volume,v","Extract volume")
    		("flight,f",po::value<unsigned>(&IDflight),"Flight number")
    		("prefix,p",po::value<string>(&pfx),"Prefix for output file names (defaults to 'out.')")
    		("case,c",po::value<unsigned>(&IDcase),"Case ID (used to filter flight selection)")
    		("runs,R",po::value<Sequence<unsigned>>(&runs),"Run(s) to extract, eg. 1-4,6,8");

    po::store(po::parse_command_line(argc,argv,cmdline),vm);
    po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
    po::notify(vm);

    boost::shared_ptr<PGConnection> dbconn;
    PGConnection::ResultType res;

    try {
        dbconn=PGConnect();
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

    vector<unsigned> runIDs;

    if(vm.count("runs"))
    {
    	if (vm.count("flight"))
    	{
    		cerr << "ERROR: Can't specify flights and runs" << endl;
    		return -1;
    	}
        vector<unsigned> runIDs=runs.as_vector();
    } else if (vm.count("flight"))
    {
    	PGConnection::ResultType res = dbconn.get()->execParams("SELECT runid FROM flights_map WHERE flightid=$1",
    			boost::tuples::make_tuple(IDflight));

    	for(int i=0;i<PQntuples(res.get()); ++i)
    	{
    		unsigned IDrun;
    		unpackPGRow(res,boost::tuples::tie(IDrun),i);
    		runIDs.push_back(IDrun);
    	}
    }
    else
    {
    	cerr << "No runs or flight specified!" << endl;
    	cerr << cmdline << endl;
    	return -1;
    }

    cout << "Runs to export (" << runIDs.size() << "): ";
    for(unsigned IDr : runIDs)
    	cout << ' ' << IDr;
    cout << endl;


    RunInfo ri = getRunInfo(dbconn.get(),runIDs[0]);

    const unsigned IDm = ri.IDmesh;
    const unsigned IDmatset = ri.IDmaterialset;

    TetraMesh *M = exportMesh(*(dbconn.get()),ri.IDmesh);

    vector<Material> mats=exportMaterials(*(dbconn.get()),ri.IDcase);

    for(unsigned IDrun : runIDs)
    {
    	ri = getRunInfo(dbconn.get(),IDrun);

    	if(ri.IDmesh != IDm)
    	{
    		cerr << "ERROR: Meshes do not match within run set" << endl;
    		return -1;
    	}
    	else if (ri.IDmaterialset != IDmatset)
    	{
    		cerr << "ERROR: Materialsets do not match within run set" << endl;
    		return -1;
    	}

    	if(vm.count("surface"))
    	{
    		stringstream ss;
    		ss << pfx << IDrun;
    		vector<double> E_s = exportResultVector(dbconn.get(),IDrun,1);
    		vector<double> phi_s = surfaceEnergyToFluence(E_s,*M);

    		writeASCII(ss.str()+".E_s.txt",E_s);
    		writeASCII(ss.str()+".phi_s.txt",phi_s);
    	}

    	if(vm.count("volume"))
    	{
    		stringstream ss;
    		ss << pfx << IDrun;
    		vector<double> E_v = exportResultVector(dbconn.get(),IDrun,2);
    		vector<double> phi_v = volumeEnergyToFluence(E_v,*M,mats);

    		writeASCII(ss.str()+".E_v.txt",E_v);
    		writeASCII(ss.str()+".phi_v.txt",phi_v);
    	}
    }
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

	vector<double>::const_iterator energy_it=energy.begin();
	vector<double>::iterator fluence_it=fluence.begin();

	for(auto E : energy | boost::adaptors::indexed(0))
		*(fluence_it++) = E.index() ? E.value() / M.getFaceArea((unsigned)E.index()) : 0;
	return fluence;
}



/** Writes out a sorted fluence vector.
 * @param begin,end		Iterator range;
 * @param N				(optional) size hint
 * @tparam Iterator		Iterator type which must dereference to const pair<unsigned,double>&
 */
/*
template<class Iterator>void writeSortedFluence_ASCII(string fn,Iterator begin,Iterator end,unsigned N=0)
{
	// make fluence into a vector
	vector<pair<unsigned,double>> fluence;
	fluence.reserve(N);

	// copy fluence out of map
	copy(begin,end,back_inserter(fluence));

	// sort descending
	sort(fluence.begin(),fluence.end(),CompareSecondDesc<unsigned,double>);

	// write to file
	ofstream os(fn.c_str());
	for(const pair<unsigned,double>& p : fluence)
		os << p.first << ' ' << p.second << endl;
	os.close();
}
*/
