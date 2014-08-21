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

template<class T,class U>bool CompareSecondDesc(const pair<T,U>& a,const pair<T,U>& b){ return a.second > b.second; }

template<class Iterator>void writeSortedFluence_ASCII(string fn,Iterator begin,Iterator end,unsigned N=0);

int main(int argc,char **argv)
{
    po::variables_map vm;
    po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
    po::notify(vm);

    boost::shared_ptr<PGConnection> dbconn;
    PGConnection::ResultType res;
    TetraMesh* m=NULL;

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


    unsigned IDm = 1;
    unsigned IDr = 2887;

    TetraMesh* M = exportMesh(*(dbconn.get()),IDm);

    FluenceMapBase* phi_s = exportResultSet(dbconn.get(),IDr,1,M);
    FluenceMapBase* phi_v = exportResultSet(dbconn.get(),IDr,2,M);

	writeSortedFluence("fluencedesc.surf.txt",phi_s->begin(),phi_s->end(),phi_s->getNNZ());
    writeSortedFluence("fluencedesc.vol.txt",phi_v->begin(),phi_v->end(),phi_v->getNNZ());
}


/** Writes out a sorted fluence vector.
 * @param begin,end		Iterator range;
 * @param N				(optional) size hint
 * @tparam Iterator		Iterator type which must dereference to const pair<unsigned,double>&
 */

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
