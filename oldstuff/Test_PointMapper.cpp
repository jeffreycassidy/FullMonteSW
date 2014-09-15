#define USE_DUMMY
#include "MeshMapper.hpp"
#include <array>
#include <vector>
#include <iostream>

#include <initializer_list>

using namespace std;

#include "sse.hpp"
#include "newgeom.hpp"

#include <boost/tuple/tuple.hpp>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>

typedef array<unsigned long,3> FaceByPointID_;
typedef array<unsigned long,4> TetraByPointID_;

int main(int argc, char **argv)
{
    vector<char> alpha(26,0);

    for(unsigned i=0;i<26;++i)
        alpha[i] = 'a'+i;

    TetraMeshDummy M;
    M.P.resize(26);

    for(unsigned i=0;i<26;++i)
    {
        array<double,3> t;
        t[0] = 100.5+i;
        t[1] = 200.5+i;
        t[2] = 300.5+i;
        M.P[i] = t;
    }

/*    for(auto it = make_indexed_iterator(alpha.begin(),1U); it != make_indexed_iterator(alpha.end(),27U); ++it)
        cout << it->get<0>() << ' ' << it->get<1>() << endl;

    vector_index_adaptor<unsigned long,char> a(alpha,0);
    for(vector_index_adaptor<unsigned long,char>::const_iterator it = a.begin(); it != a.end(); ++it)
        cout << it->get<0>() << ' ' << it->get<1>() << endl;


    for(const boost::tuple<unsigned,char>& v : a)
        cout << v.get<0>() << ' ' << v.get<1>() << endl;

    for(auto const& v : a)
        cout << v.get<0>() << ' ' << v.get<1>() << endl;

    for(auto const& v : vector_index_adaptor<unsigned long,char>(alpha,0))
        cout << v.get<0>() << ' ' << v.get<1>() << endl;*/


    /* Test by remapping points 1,3 to zero, ie. deleting them from range */


    cout << "Remapping points to delete 1,3" << endl;
    vector<unsigned long> lut(100,0);
    for(unsigned i=0; i<100; ++i)
        lut[i]=i;

    lut[0]=1;
    lut[1]=0;
    lut[3]=0;

    MeshMapping::PointMapper pm(M,lut);

    for(auto it : pm)
        cout << it.get<0>() << ": " << it.get<1>() << endl;



    cout << "Remapping faces to delete 5,6 and apply point mapping" << endl;
    vector<unsigned long> flut(10,0);
    for(unsigned i=0; i<10; ++i)
        flut[i]=i;

    lut[92]=0;
    lut[10]=101;
    lut[11]=1001;
    lut[12]=10001;

    flut[5]=0;
    flut[6]=0;

    MeshMapping::FaceMapper fm(M,lut,flut);

    M.F.resize(10);
    for(unsigned i=0; i<10; ++i)
    {
        M.F[i][0] = 10*i;
        M.F[i][1] = 10*i+1;
        M.F[i][2] = 10*i+2;
    }

    for(auto it : fm)
        cout << it.get<0>() << ": " << it.get<1>() << endl;
}
