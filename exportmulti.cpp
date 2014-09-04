#include <iostream>
#include <iomanip>
#include "fm-postgres/fm-postgres.hpp"

#include "fm-postgres/fmdbexportcase.hpp"
#include "fmdb.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

#include <boost/numeric/ublas/matrix.hpp>

namespace po=boost::program_options;
using namespace std;

#include "Sequence.hpp"


namespace __cmdline_opts {
	Sequence<unsigned> runs;
	string ofile;
};

int main(int argc,char **argv)
{
    // define command-line options
    po::options_description cmdline("Command-line options");
    po::positional_options_description pos;
    pos.add("runs",1).add("outfile",1);


    cmdline.add_options()
        ("help,h","Display option help")
        ("runs,r",po::value<Sequence<unsigned>>(&__cmdline_opts::runs),"RNG seed (int)")
        ("outfile,o",po::value<string>(&__cmdline_opts::ofile),"Outfile file name")
        ;

    cmdline.add(globalopts::db::dbopts);

    // parse options (if an option is already set, subsequent store() calls will not overwrite
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc,argv).options(cmdline).positional(pos).run(),vm);
        po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
        po::notify(vm);
    }
    catch (po::error& e){
        cerr << "Caught an exception in options processing" << endl;
        cout << cmdline << endl;
        return -1;
    }

    // catch case where we just print help and exit
    if (vm.count("help"))
    {
    	cout << cmdline << endl;
    	return 0;
    }

    cout << "Outfile file: " << __cmdline_opts::ofile << endl;
    cout << "Runs:";

    vector<unsigned> v = __cmdline_opts::runs.as_vector();
    for(unsigned r : v)
    	cout << ' ' << r;
    cout << endl;


    // make the database connection
    cout << "DB host: " << globalopts::db::host << endl;
    cout << "DB port: " << globalopts::db::port << endl;
    cout << "DB name: " << globalopts::db::name << endl;

    boost::shared_ptr<PGConnection> dbconn;

    try {
    	dbconn = PGConnect();
    }
    catch(PGConnection::PGConnectionException& e)
    {
    	cerr << "Failed to connect with error: " << e.msg << endl;
    	return -1;
    }


    // do the export into a matrix of size (Nt, Nruns)
    unsigned long Nt=307000;
    unsigned long dtype=2;
    boost::numeric::ublas::matrix<double> M(Nt,v.size());

    unsigned j=0;
    for(unsigned IDr : v)
    {
    	FluenceMapBase *fmap = exportResultSet(dbconn.get(),IDr,dtype);
    	cout << "  Run " << IDr << " with " << fmap->getNNZ() << " nonzero elements" << flush;
    	double sum=0.0;

    	if(!fmap)
    	{
    		cerr << "Invalid run; no data returned" << endl;
    				return -1;
    	}

    	// copy elementwise from the sparse iterator
    	unsigned N=0;
    	for(FluenceMapBase::const_iterator it=fmap->begin(); it != fmap->end(); ++it)
    	{
    		sum += it->second;
    		++N;
    		if (it->first >= Nt)
    			cerr << "Invalid element index " << it->first << ">" << Nt << endl;
    		else
    			M(it->first,j) = it->second;
    	}
    	j++;

    	cout << ": " << sum << " (" << N << ')'<< endl;

    	delete fmap;
    }

    // check column sums
    j=0;
    for(boost::numeric::ublas::matrix<double>::iterator2 c_it=M.begin2(); c_it != M.end2(); ++c_it,++j)
    {
    	double sum=0.0;
    	for(auto it = c_it.begin(); it != c_it.end(); ++it)
    		sum += *it;
    	cout << "  Column " << j << " sum is " << setprecision(6) << sum << endl;
    }

    // write matrix into .mat file
	return 0;
}
