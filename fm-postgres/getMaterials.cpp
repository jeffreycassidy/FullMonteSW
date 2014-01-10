#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <source.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "fm-postgres.hpp"

#include "io_timos.hpp"
#include "source.hpp"
#include "graph.hpp"

using namespace std;

int main(int argc,char **argv)
{
    vector<Material> materials;

    unsigned IDc=1;
    if (argc > 1)
        IDc=atoi(argv[1]);

    boost::shared_ptr<PGConnection> dbconn = PGConnect();
	//(connprops_default);

    try {
        exportMaterials(*(dbconn.get()),IDc,materials);
    }
    catch(PGConnection::PGConnectionException e)
    {
        cerr << e.msg << endl;
    }

    cout << setw(8) << "mu_t" << setw(8) << "1/mu_t" << setw(8) << "g" << setw(8) << "absfrac" << endl;
    for(vector<Material>::const_iterator it=materials.begin(); it != materials.end(); ++it)
    {
        cout << setw(8) << hex << (unsigned)(it->getMuT()*(1<<6));
        cout << setw(8) << hex << (unsigned)(1/it->getMuT()*(1<<12));
        cout << setw(8) << hex << (unsigned)(it->getg()*(1<<18));
		cout << endl;
    }
}
