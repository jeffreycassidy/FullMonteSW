//#include "io_nirfast.hpp"

#include "newgeom.hpp"
#include "graph.hpp"
#include "TriMesh.hpp"

#include "linefile.hpp"

#include <boost/graph/adjacency_list.hpp>

vector<unsigned> nirfast_getRegions(string fn_root);
vector<Point<3,double> > nirfast_getPoints(string fn_root);
vector<TetraByPointID> nirfast_getTetras(string fn_root);
TetraMesh nirfast_load(string fn_root);

pair<unsigned,unsigned> edgePair(unsigned a,unsigned b)
{
    return (a < b ? make_pair(a,b) : make_pair(b,a));
}

unsigned greedySearch(const TetraMesh& mesh,unsigned** el,Point<3,double> P,unsigned v_start=1)
{
    unsigned v_curr=v_start,v_next=v_start,N_step=0,N_cmp=0;
    Point<3,double> P_nearest=mesh.getPoint(v_start);
    double d_min_last = Vector<3,double>(P,P_nearest).norm_l2(),d_min = std::numeric_limits<double>::infinity(),d;

    do {
        ++N_step;
        d_min=d_min_last;
        v_curr=v_next;
        for(unsigned i=1;i <= el[v_curr][0]; ++i)
        {
            ++N_cmp;
            if((d=Vector<3,double>(P,mesh.getPoint(el[v_curr][i])).norm2()) < d_min)
            {
                d_min = d;
                v_next = el[v_curr][i];
            }
        }
    }
    while(d_min < d_min_last);

    cout << "Searching for " << P << " , located point " << mesh.getPoint(v_curr) << " after " << N_step << " nodes visited, " <<
        N_cmp << " comparisons" << endl;

    return v_next;
}

int main(int argc,char **argv)
{
//    boost::graph::adjacency_list<vecS,vecS,bidirectionalS,no_property,no_property,no_property,listS> graph;
    TetraMesh mesh=nirfast_load("/Users/jcassidy/Downloads/Volume_XPM/Volume_XPM");

    set<pair<unsigned,unsigned> > edgeSet;

    cout << "Inserting " << 12*mesh.getNt() << " edge pairs from " << mesh.getNt() << " tetras" << endl;

    // create the point graph by looping over all tetras and adding all edges
    for(TetraMesh::tetra_const_iterator it=mesh.tetraIDBegin(); it != mesh.tetraIDEnd(); ++it)
    {
        edgeSet.insert(make_pair((*it)[0],(*it)[1]));
        edgeSet.insert(make_pair((*it)[0],(*it)[2]));
        edgeSet.insert(make_pair((*it)[0],(*it)[3]));
        edgeSet.insert(make_pair((*it)[1],(*it)[2]));
        edgeSet.insert(make_pair((*it)[1],(*it)[3]));
        edgeSet.insert(make_pair((*it)[2],(*it)[3]));

        edgeSet.insert(make_pair((*it)[1],(*it)[0]));
        edgeSet.insert(make_pair((*it)[2],(*it)[0]));
        edgeSet.insert(make_pair((*it)[3],(*it)[0]));
        edgeSet.insert(make_pair((*it)[2],(*it)[1]));
        edgeSet.insert(make_pair((*it)[3],(*it)[1]));
        edgeSet.insert(make_pair((*it)[3],(*it)[2]));
    }

    cout << "Created edge set with total of " << edgeSet.size() << " distinct edges" << endl;

    // Create adjancency graph
    unsigned** ptr      = new unsigned*[mesh.getNp()+1];
    unsigned*  edgeList = new unsigned[edgeSet.size() + mesh.getNp()+1];

    set<pair<unsigned,unsigned> >::const_iterator s_it=edgeSet.begin();

    for(unsigned i=0;i<mesh.getNp(); ++i)
    {
        ptr[i] = edgeList++;
        for(ptr[i][0] = 0; s_it->first==i; ++ptr[i][0])
            (*edgeList++) =(*s_it++).second;
    }

    cout << "First 10 graph nodes: " << endl;
    for(unsigned i=0;i<10;++i)
    {
        cout << i << ": ";
        for(unsigned j=1;j<=ptr[i][0];++j)
        {
            cout << ptr[i][j] << ' ';
        }
        cout << endl;
    }

    double p0[3] = { 0,0,0 };

    unsigned tmp = greedySearch(mesh,edgeList,Point<3,double>(p0),1);

    // load the triangular mesh
    TriMesh tri;

    tri.fromVTKFile("rw-fudge.vtk");

    tri.matchTo(mesh);

    delete[] ptr[0];
    delete[] ptr;
}

TetraMesh nirfast_load(string fn_root)
{
    vector<TetraByPointID> T_p;
    vector<unsigned> T_m;
    vector<Point<3,double> > P;

    T_p = nirfast_getTetras(fn_root+".elem");
    T_m = nirfast_getRegions(fn_root+".region");
    P   = nirfast_getPoints(fn_root+".node");
    
    return TetraMesh(P,T_p,T_m);
}


// loads the .elem file
vector<TetraByPointID> nirfast_getTetras(string fn)
{
    vector<TetraByPointID> tetras(1);
    TetraByPointID IDps;
    ifstream is (fn.c_str(),ios_base::in);
    if (!is.good()){
        cerr << "Failed to open file" << endl;
        return vector<TetraByPointID>();
    }

    unsigned pmin=-1,pmax=0;

    while(!is.eof())
    {
        is >> IDps;
        if (!is.eof())
        {
            pmin = min(pmin,IDps.getMin());
            pmax = max(pmax,IDps.getMax());
            tetras.push_back(IDps);
        }
    }

    cout << "Loaded " << tetras.size()-1 << " tetrahedra, point indices " << pmin << '-' << pmax << endl;
    return tetras;
}

vector<Point<3,double> > nirfast_getPoints(string fn)
{
    vector<Point<3,double> > P(1);
    Point<3,double> p;
    ifstream is(fn.c_str(),ios_base::in);
    if (!is.good())
    {
        cerr << "Failed to open file" << endl;
        return vector<Point<3,double> >();
    }
    unsigned t;

    while (!is.eof())
    {
        is >> t >> p;
        if(!is.eof())
            P.push_back(p);
    }

    cout << "Loaded " << P.size()-1 << " points" << endl;
    return P;
}

vector<unsigned> nirfast_getRegions(string fn)
{
    vector<unsigned> mat(1);
    unsigned m,matmin=-1,matmax=0;
    ifstream is(fn.c_str(),ios_base::in);
    if (!is.good())
    {
        cerr << "Failed to open file" << endl;
        return vector<unsigned>();
    }

    while(!is.eof())
    {
        is >> m;
        if (!is.eof())
        {
            matmin=min(matmin,m);
            matmax=max(matmax,m);
            mat.push_back(m);
        }
    }

    cout << "Loaded " << mat.size()-1 << " regions, range " << matmin << "-" << matmax << endl;
    return mat;
}
