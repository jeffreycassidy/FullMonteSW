#include <vector>
#include <map>
#include "../newgeom.hpp"
#include <iostream>
#include <fstream>
#include "../helpers.hpp"

#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>

using namespace std;
using namespace boost;

// f(x)=x
/*class MapperIdentity {
    public:
    double operator()(double x) const { return x; }
};

// f(X) = X_i
class MapperIndex {
    int idx;
    public:
    MapperIndex(int idx_) : idx(idx_){};
    double operator()(double* x) const { return x[idx]; }
};

// f(x) = k*x + c
class MapperScaleOffset {
    double k,c;
    public:
    MapperScaleOffset(double k_,double c_=0.0) : k(k_),c(c_){};
    double operator()(double x) const { return k*x+c; }
};

class MapperLog {
    double xmin;
    public:
    double MapperLog(double xmin_) : xmin(xmin_) {};
    double operator()(double x) const { return log(x < xmin ? xmin : x); }
};*/

// Writes out a VTK datafile (v3.0)
// Copy of LoggerSurface::writeFileVTK(string fn) from logSurface.cpp

template<class T,class U>U first(const pair<T,U>& p)
{
    return p.first;
}

template<class T,class U>U second(const pair<T,U>& p)
{
    return p.second;
}

void writeFileVTK(string fn,const vector<Point<3,double> >& P,const map<FaceByPointID,double>& F)
{
    ofstream os(fn.c_str());

    Stats s = for_each(
        make_transform_iterator(F.begin(),second<FaceByPointID,double>),
        make_transform_iterator(F.end(),  second<FaceByPointID,double>),
        Stats());
    cout << "INFO: Emittance " << s << endl;

    // file header
    os << "# vtk DataFile Version 3.0" << endl;
    os << "(comment)" << endl;
    os << "ASCII" << endl;
    os << "DATASET POLYDATA" << endl;

    // point data
    os << "POINTS " << P.size()-1 << " float" << endl;
    for(vector<Point<3,double> >::const_iterator it=P.begin()+1; it != P.end(); ++it)
        os << (*it)[0] << ' ' << (*it)[1] << ' ' << (*it)[2] << endl;

    // surface mesh triangles
    // subtract 1 from all indices to comply with VTK zero-based indexing
    os << "POLYGONS " << F.size() << " " << F.size()*4 << endl;
    for(map<FaceByPointID,double>::const_iterator it=F.begin(); it != F.end(); ++it)
    {
        FaceByPointID f = it->first;
        os << "3 " << f[0]-1 << ' ' << f[1]-1 << ' ' << f[2]-1 << endl;
    }

    // emittance
    os << "CELL_DATA " << F.size() << endl;
    os << "SCALARS emittance float 1" << endl;
    os << "LOOKUP_TABLE default" << endl;

    for(map<FaceByPointID,double>::const_iterator it=F.begin(); it != F.end(); ++it)
        os << it->second << endl;
}
