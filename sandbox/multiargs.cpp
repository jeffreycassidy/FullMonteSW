#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

#include <vector>
using namespace std;

vector<unsigned> suites,cases;


int main(int argc,char **argv)
{
    boost::program_options::options_description cmdline("Command-line options");

    cmdline.add_options()
        ("suites,s",boost::program_options::value<vector<unsigned> >(&suites))
        ("cases,c",boost::program_options::value<vector<unsigned> >(&cases));

    boost::program_options::variables_map vm;

    boost::program_options::store(boost::program_options::command_line_parser(argc,argv).options(cmdline).run(),vm);
    boost::program_options::notify(vm);

    cout << "Suites: ";
    for(vector<unsigned>::const_iterator it=suites.begin(); it != suites.end(); ++it)
        cout << *it << ' ';
    cout << endl << "Cases: ";
    for(vector<unsigned>::const_iterator it=cases.begin(); it != cases.end(); ++it)
        cout << *it << ' ';
    cout << endl;
}
