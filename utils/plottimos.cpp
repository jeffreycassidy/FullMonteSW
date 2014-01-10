#include "../graph.hpp"
#include "../newgeom.hpp"
#include "../io_timos.hpp"
#include <iostream>
#include <map>

using namespace std;

void writeFileVTK(string fn,const vector<Point<3,double> >& P,const map<FaceByPointID,double>& F);

namespace globalopts {
    bool matched_boundary=false;
};

int main(int argc,char **argv)
{
    if (argc != 4)
    {
        cerr << "Needs exactly three arguments: <mesh-file> <result-file> <output-file>" << endl;
        return -1;
    }

    // load geometry with dummy materials
    TetraMesh M(argv[1],TetraMesh::MatlabTP);

    // load surface def
//    map<FaceByPointID,double*> timos;
//    double *data = loadTIMOSSurf(argv[2],timos);

    map<FaceByPointID,double> timos_0;
    loadTIMOSSurf(argv[2],timos_0);

//    for(map<FaceByPointID,double*>::const_iterator it=timos.begin(); it != timos.end(); ++it)
//        timos_0.insert(make_pair(it->first,*(it->second)));

    writeFileVTK(argv[3],M.getPoints(),timos_0);

//    delete[] data;
    return 0;
}

