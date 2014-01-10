#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <source.hpp>
#include "fm-postgres.hpp"

#include "io_timos.hpp"
#include "source.hpp"
#include "graph.hpp"

using namespace std;

int main(int argc,char **argv)
{
    vector<Source*> sources;
    vector<Material> materials;

    unsigned IDc=1;
    if (argc > 1)
        IDc=atoi(argv[1]);

    cout << "Attempting" << endl;

    PGConnection dbconn(connprops_default);

    cout << " connected" << endl;

    TetraMesh *m;

    try {
        m = exportMesh(dbconn,IDc);
        exportSources(dbconn,IDc,sources);
        exportMaterials(dbconn,IDc,materials);
    }
    catch(PGConnection::PGConnectionException e)
    {
        cerr << e.msg << endl;
    }

    writeTIMOSMaterials("output.opt",materials);
    writeTIMOSSource("output.source",sources);
    if (!m->writeFileMatlabTP("output.mesh"))
        cerr << "Failed to write mesh" << endl;
}
