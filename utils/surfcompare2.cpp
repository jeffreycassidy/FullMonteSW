#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include "fluencemap.hpp"
#include "../newgeom.hpp"
#include "../io_timos.hpp"

using namespace std;

int main(int argc,char **argv)
{
    if (argc < 6)
    {
        cerr << "Needs five arguments: surfcompare <timos-mesh-file> <timos-result-file> <surf-file>" << endl;
        return -1;
    }

    FluenceMap a,b;

    TetraMesh M(argv[1],TetraMesh::MatlabTP);

    a.readTIMOS(argv[2],M);
    b.readASCIIOutput(argv[3]);

    FluenceMap diff(a);
    diff -= b;

    return 0;
}
