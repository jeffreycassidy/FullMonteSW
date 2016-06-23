#include <mmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <iostream>

#include <boost/timer/timer.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_01.hpp>

#include "../newgeom.hpp"
#include "../graph.hpp"
#include "../source.hpp"

#include "timebench.hpp"

using namespace std;

ostream& operator<<(ostream& os,const StepResult& r)
{
    os << (r.idx!=-1 ? 'T' : 'F') << " idx=" << setw(3) << (int)r.idx << " IDfe=" << setw(6) << r.IDfe << " IDte=" << setw(6) << r.IDte << " dist=" << r.distance << " Pe=" << r.Pe;
    return os;
}

bool operator==(const StepResult& a,const StepResult& b)
{
    return ((a.idx==-1 && b.idx==-1) || (a.idx==b.idx &
        a.IDfe == b.IDfe &
        a.IDte == b.IDte &
        fabs(getFloat(a.distance)-getFloat(b.distance)) < 1e-3 & 
        getFloat(norm(a.Pe-b.Pe)) < 1e-3));
}

typedef struct {
    Tetra tet;
    __m128 p,d;
    double s;
} Input;

void runReference(const Input* i,StepResult* o)
{
    float tmp[4] __attribute__((aligned(16)));

    __m128 nx=i->tet.nx;
    __m128 ny=i->tet.ny;
    __m128 nz=i->tet.nz;
    __m128 C =i->tet.C;

    __m128 p = i->p;
    __m128 d = i->d;

    // calculate dot = n (dot) d, height = n (dot) p - C
    __m128 dot    =             _mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0)));
    __m128 height =             _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));

    dot    = _mm_add_ps(dot,    _mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))));
    height = _mm_add_ps(height, _mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));

    dot    = _mm_add_ps(dot,    _mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2))));
    height = _mm_add_ps(height, _mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));

    // height (=C - d dot n) should be negative if inside tetra, may occasionally be positive due to numerical error
    // dot negative means facing outwards
    height = _mm_sub_ps(C,height);

    // dist = height/dot
    __m128 dist   = _mm_div_ps(height,dot);
    dist = _mm_and_ps(dist,_mm_cmpgt_ps(_mm_setzero_ps(),dot));

    // at most three of the dot products should be negative
    // ideally none of the heights should be negative (assuming we're in the tetra)

    //      height  dot     h/dot   meaning
    //      +       +       +       OK: inside, facing away (no possible intersection)
    //      +       -       -       OK: inside, facing towards (intersection possible)
    //      -       +       -       OK: outside, facing in (this is the entry face with roundoff error, no possible intersection)
    //      -       -       +       ERROR: outside, facing out (problem!! this must be the entry face, but exiting!)

    // require p dot n - C > 0 (above face) and d dot n < 0
    unsigned min_i=-1;
    float min_d=std::numeric_limits<float>::infinity();
    _mm_store_ps(tmp,dist);
    for(unsigned j=0;j<4;++j)
        if(tmp[j] < min_d && tmp[j] > 0)
            min_d=tmp[j],min_i=j;

// need: minimum distance, endpoint

//    if (min_i > 3)
//        cout << "In getIntersection with dist=" << dist << endl;

    if (min_d < i->s)
    {
        o->distance = _mm_set1_ps(min_d);
        o->IDfe = i->tet.IDfs[min_i];
        o->IDte = i->tet.adjTetras[min_i];
        o->idx = min_i;

		// limit cos(theta) to one to avoid NaNs when computing sin theta
    }
    else
    {
        o->distance = _mm_set1_ps(i->s);
        o->idx=-1;
    }
    
    o->Pe = _mm_add_ps(p,_mm_mul_ps(d,o->distance));
}


/*
inline void runTest(const Input* i,StepResult *o)
{
    __m128 p=i->p;
    __m128 d=i->d;
    double s=i->s;

    // pe = [-1 pe_z pe_y pe_x]
    __m128 pe = _mm_or_ps(_mm_set_ps(0,0,0,-1),_mm_add_ps(p,_mm_mul_ps(_mm_set1_ps(s),d)));

    // calc (pe dot n - C)
    __m128 dot01 = _mm_hadd_ps(_mm_mul_ps(i->Fn[0],pe),_mm_mul_ps(i->Fn[1],pe));
    __m128 dot23 = _mm_hadd_ps(_mm_mul_ps(i->Fn[2],pe),_mm_mul_ps(i->Fn[3],pe));

    __m128 h_final = _mm_hadd_ps(dot01,dot23);

    o->intersection = _mm_movemask_ps(h_final) != 0;

    if (o->intersection)
    {
    }
    else
        o->Pe = pe;
}*/

inline void runNull(const Input* i,StepResult *o)
{
    o->Pe = i->p;
    o->distance = _mm_set1_ps(1.0);
//    o->intersection = _mm_movemask_ps(i->tet.Fn[0]);
}
/*
inline void runTest(const Input* i,StepResult *o)
{
    __m128 d = i->d;
    __m128 p = i->p;
    __m128 min_d;
    __m128 s=_mm_set1_ps(i->s);
    unsigned min_i;
    pair<unsigned,__m128> p_tmp;

    __m128 nx = i->tet.Fn[0];
    __m128 ny = i->tet.Fn[1];
    __m128 nz = i->tet.Fn[2];
    __m128 C  = i->tet.Fn[3];

    _MM_TRANSPOSE4_PS(nx,ny,nz,C);

    // calculate dot = n (dot) d, height = n (dot) p - C
    __m128 dotprod    =             _mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0)));
    __m128 height =             _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));

    dotprod    = _mm_add_ps(dotprod,    _mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))));
    height = _mm_add_ps(height, _mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));

    dotprod    = _mm_add_ps(dotprod,    _mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2))));
    height = _mm_add_ps(height, _mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));

    // height (=C - d dot n) should be negative if inside tetra, may occasionally be positive due to numerical error
    // dot negative means facing outwards so we set height to be negative as well for positive distance
    height = _mm_sub_ps(C,height);

    // dist = height/dotprod
    __m128 dist   = _mm_div_ps(height,dotprod);
    __m128 inf    = _mm_set1_ps(std::numeric_limits<float>::infinity());

    // sets distance for points with positive dot product (facing inwards) to infinity
    dist = _mm_blendv_ps(inf,dist,dotprod);

    // at most three of the dot products should be negative
    // ideally none of the heights should be negative (assuming we're in the tetra)

    //      height  dot     h/dot   meaning
    //      +       +       +       OK: inside, facing away (no possible intersection)
    //      +       -       -       OK: inside, facing towards (intersection possible)
    //      -       +       -       OK: outside, facing in (this is the entry face with roundoff error, no possible intersection)
    //      -       -       +       ERROR: outside, facing out (problem!! this must be the entry face, but exiting!)

    // require p dot n - C > 0 (above face) and d dot n < 0
    p_tmp = getMinIndex4p(dist);

    min_i=p_tmp.first;
    min_d=p_tmp.second;

// need: minimum distance, endpoint

    o->distance=min_d;
    o->intersection = _mm_ucomilt_ss(o->distance,s);
    if (o->intersection)
    {
        o->IDfe = i->tet.IDfs[min_i];
        o->IDte = i->tet.adjTetras[min_i];

        // cos(theta) = -d dot n
//		__m128 one = _mm_set_ss(1.0);

		// limit cos(theta) to one to avoid NaNs when computing sin theta
//        o->costheta = _mm_min_ss(one,_mm_sub_ss(_mm_setzero_ps(),selectFrom_v(dot,min_i)));

        o->n = i->tet.Fn[min_i];
        o->costheta = _mm_min_ps(_mm_set1_ps(1.0),_mm_sub_ps(_mm_setzero_ps(),dot4bcast(o->n,d)));
    }
    
    o->Pe = _mm_add_ps(p,_mm_mul_ps(d,min_d));
}*/

/*

inline void runTest2(const Input* i,StepResult *o)
{
    __m128 d = i->d;
    __m128 p = i->p;
    __m128 min_d;
    double s=i->s;
    unsigned min_i;
    pair<unsigned,__m128> p_tmp;

    __m128 p1 = _mm_add_ps(_mm_set_ps(0,0,0,-1),p);

    __m128 halfdot01 = _mm_hadd_ps(_mm_mul_ps(i->tet.Fn[0],d),_mm_mul_ps(i->tet.Fn[1],d));
    __m128 halfdot23 = _mm_hadd_ps(_mm_mul_ps(i->tet.Fn[2],d),_mm_mul_ps(i->tet.Fn[3],d));
    __m128 dotprod = _mm_hadd_ps(halfdot01,halfdot23);

    __m128 halfheight01 = _mm_hadd_ps(_mm_mul_ps(i->tet.Fn[0],p1),_mm_mul_ps(i->tet.Fn[1],p1));
    __m128 halfheight23 = _mm_hadd_ps(_mm_mul_ps(i->tet.Fn[2],p1),_mm_mul_ps(i->tet.Fn[3],p1));
    __m128 height = _mm_hadd_ps(halfheight01,halfheight23);

    // height (=C - d dot n) should be negative if inside tetra, may occasionally be positive due to numerical error
    // dot negative means facing outwards so we set height to be negative as well for positive distance

    // dist = height/dotprod
    __m128 dist   = _mm_div_ps(height,dotprod);
    __m128 inf    = _mm_set1_ps(std::numeric_limits<float>::infinity());

    // sets distance for points with positive dot product (facing inwards) to infinity
    dist = _mm_blendv_ps(inf,dist,dotprod);

    p_tmp = getMinIndex4p(dist);

    min_i=p_tmp.first;
    min_d=p_tmp.second;

// need: minimum distance, endpoint

    o->distance=min_d;
    o->intersection = _mm_ucomilt_ss(o->distance,_mm_set_ss(s));
    if (o->intersection)
    {
        o->IDfe = i->tet.IDfs[min_i];
        o->IDte = i->tet.adjTetras[min_i];

        o->n = i->tet.Fn[min_i];
        o->costheta = _mm_min_ps(_mm_set1_ps(1.0),_mm_sub_ps(_mm_setzero_ps(),dot4bcast(o->n,d)));
    }
    
    o->Pe = _mm_add_ps(p,_mm_mul_ps(d,min_d));
}*/

inline void runTest_New(const Input* i,StepResult *o)
{
    *o = i->tet.getIntersection(i->p,i->d,_mm_set1_ps(i->s));
}

/*inline void runTest3(const Input* i,StepResult* o)
{
    __m128 nx = i->tet.Fn[0];
    __m128 ny = i->tet.Fn[1];
    __m128 nz = i->tet.Fn[2];
    __m128 C  = i->tet.Fn[3];

    __m128 p = i->p;
    __m128 d = i->d;

    __m128 s = _mm_set1_ps(i->s);

    _MM_TRANSPOSE4_PS(nx,ny,nz,C);

    // calculate dot = n (dot) d, height = n (dot) p - C
    __m128 dot    =             _mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0)));
    __m128 height =             _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));

    dot    = _mm_add_ps(dot,    _mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))));
    height = _mm_add_ps(height, _mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));

    dot    = _mm_add_ps(dot,    _mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2))));
    height = _mm_add_ps(height, _mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));

    // height (=C - d dot n) should be negative if inside tetra, may occasionally be (small) positive due to numerical error
    // dot negative means facing outwards
    height = _mm_sub_ps(C,height);

    // dist = height/dot
    __m128 dist   = _mm_div_ps(height,dot);

//  selects dist where dot<0, s otherwise
    dist = _mm_blendv_ps(s,dist,_mm_and_ps(_mm_cmpgt_ps(dist,_mm_setzero_ps()),dot));
//    dist = _mm_max_ps(_mm_setzero_ps(),dist);

    // at most three of the dot products should be negative
    // ideally none of the heights should be negative (assuming we're in the tetra)

    //      height  dot     h/dot   meaning
    //      +       +       +       OK: inside, facing away (no possible intersection)
    //      +       -       -       OK: inside, facing towards (intersection possible)
    //      -       +       -       OK: outside, facing in (this is the entry face with roundoff error, no possible intersection)
    //      -       -       +       ERROR: outside, facing out (problem!! this must be the entry face, but exiting!)

    // require p dot n - C > 0 (above face) and d dot n < 0

    pair<unsigned,__m128> min_idx_val = getMinIndex4p(dist);

    o->intersection = _mm_ucomilt_ss(min_idx_val.second,s);
    if (o->intersection)
    {
        o->IDfe = i->tet.IDfs[min_idx_val.first];
        o->IDte = i->tet.adjTetras[min_idx_val.first];
        o->idx = min_idx_val.first;

        // cos(theta) = -d dot n
		__m128 one = _mm_set_ss(1.0);

		// limit cos(theta) to one to avoid NaNs when computing sin theta
        o->n = i->tet.Fn[min_idx_val.first];
        o->costheta = _mm_min_ss(one,_mm_sub_ps(_mm_setzero_ps(),dot4bcast(o->n,d)));
    }
    o->distance=min_idx_val.second;
    o->Pe = _mm_add_ps(p,_mm_mul_ps(d,o->distance));
}*/

const unsigned N=1000000;
const unsigned Nt=1000;

vector<VolumeSource*> src;

//RNG_SFMT rng;
//boost::random::uniform_01<double> uni01;
extern boost::random::uniform_01<double> uni01;
boost::random::uniform_int_distribution<unsigned> uni(1,Nt);

TetraMesh *M;

Input genRandStim()
{
    unsigned IDt=uni(rng);
    Input i;
    Packet pkt;

    pkt = src[IDt-1]->emit().first;

    i.p = pkt.p;
    i.d = pkt.d;
    i.tet = M->getTetra(IDt);
    i.s = -logf(1-uni01(rng))/20.0;

    return i;
}

int main(int argc,char **argv)
{
    Result r;

    cout << "Loading digimouse - ";
    M = new TetraMesh(string("../../../../data/TIM-OS/mouse/mouse.mesh"),TetraMesh::MatlabTP);
    cout << "OK" << endl;

    src.resize(Nt);

    // create Nt sources from the first Nt tetras in the geometry
    for(unsigned i=0;i<Nt;++i)
    {
        src[i] = new VolumeSource(i+1);
        src[i]->prepare(*M);
    }

    cout << "Sources prepared" << endl;

    TimeBench<Input,StepResult> tb(runReference,N,default_comp<StepResult,StepResult>());
    tb.makeStimulus(genRandStim);

    cout << " == Reference ========================================" << endl;
    r = tb.run(runReference);
    cout << "user time=" << r.t_user*1e6 << "us errors=" << r.Nerr << "/" << N << endl;

    cout << " == TestNew  ========================================" << endl;
    r = tb.run(runTest_New);
    cout << "user time=" << r.t_user*1e6 << "us errors=" << r.Nerr << "/" << N << endl;

//    cout << " == Test2     ========================================" << endl;
//    r = tb.run(runTest2);
//    cout << "user time=" << r.t_user*1e6 << "us errors=" << r.Nerr << "/" << N << endl;

//    cout << " == Test3     ========================================" << endl;
//    r = tb.run(runTest3);
//    cout << "user time=" << r.t_user*1e6 << "us errors=" << r.Nerr << "/" << N << endl;
//    r = tb.run(runNull);
//    cout << "user time=" << r.t_user*1e6 << "us errors=" << r.Nerr << "/" << N << endl;
}
