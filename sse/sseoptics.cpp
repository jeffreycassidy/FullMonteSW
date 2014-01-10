#include "../optics.hpp"
#include "timebench.hpp"

#include <pmmintrin.h>

#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

#include "../newgeom.hpp"

extern boost::random::mt19937 rng;
extern boost::random::uniform_01<double> uni01;


template<int D,class T>class VectorCompare {
    T eps2;
    public:
    VectorCompare(T eps_) : eps2(eps_*eps_){}

//    bool operator==(const Point<D,T>& a) const { return norm2_l2(a-val) < eps2; }

    bool operator()(const Vector<D,T>& a,const Vector<D,T>& b) const {
        double diff = norm2_l2(Vector<D,T>(a-b));
        return diff<eps2; }
};


typedef struct {
    double n1;
    double n2;
    UnitVector<2,double> vi;
} RefractInput;

void RefractBase(const RefractInput* i,UnitVector<2,double>* o)
{
    *o = Refract(i->n1,i->n2,i->vi);
}

typedef struct {
    __m128 n1_n2_ratio;
//    __m128 n;           // normal vector
//    __m128 p;           // in-plane (parallel) vector
    __m128 sincos;      // r0=cos(theta) r1=sin(theta)
} RefractSSEInput;

RefractInput RefractStim()
{
    RefractInput i;
    double theta,tmp[2];

    do {
        theta = boost::math::constants::pi<double>()/2.0*uni01(rng);
        i.n1=1.0+uni01(rng);
        i.n2=1.0+uni01(rng);
        tmp[0]=cos(theta);
        tmp[1]=sin(theta);
        i.vi = UnitVector<2,double>(tmp,true);
    }
    while(i.n1*i.vi[1] > i.n2);

    return i;
}


void RefractReference(const RefractInput* i,UnitVector<2,double>* o)
{
    *o = Refract(i->n1,i->n2,i->vi);
}

void RefractSSEWrap(const RefractSSEInput* i,__m128* o)
{
    *o = RefractSSE(i->n1_n2_ratio,i->sincos);
}

class RefractSSEMap {
    public:
    typedef RefractInput    input_type;
    typedef RefractSSEInput result_type;
    RefractSSEInput operator()(const RefractInput&);
};

class RefractSSEOMap {
    public:
    typedef __m128      input_type;
    typedef UnitVector<2,double> result_type;
    UnitVector<2,double> operator()(__m128);
};

UnitVector<2,double> RefractSSEOMap::operator()(__m128 v)
{
    float f[4];
    double d[2];
    _mm_store_ps(f,v);
    d[0]=f[3];
    d[1]=f[2];
    return UnitVector<2,double>(d,true);
}

RefractSSEInput RefractSSEMap::operator()(const RefractInput& i)
{
    RefractSSEInput t;
    t.n1_n2_ratio = _mm_set_ps(0,i.n1/i.n2,i.n2,i.n1);
    t.sincos = _mm_set_ps(0,0,i.vi[0],i.vi[1]);
    return t;
}

typedef struct {
    double n1,n2;
    UnitVector<2,double> vi,vt;
} __attribute__((aligned(64))) FresnelInput;

void FresnelBase(const FresnelInput* i,double* o)
{
    *o = FresnelReflect(i->n1,i->n2,i->vi,i->vt);
}

typedef struct {
    __m128 n1n2;    // r0=n1 r1=n2
    __m128 costheta;
} __attribute__((aligned(64))) FresnelSSEInput;

class FresnelSSEMap {
    public:
    typedef FresnelInput    input_type;
    typedef FresnelSSEInput result_type;
    FresnelSSEInput operator()(const FresnelInput&);
};

FresnelSSEInput FresnelSSEMap::operator()(const FresnelInput& i)
{
    FresnelSSEInput t;
    t.n1n2  = _mm_set_ps(0,i.n2/i.n1,i.n2,i.n1);
    t.costheta = _mm_set_ss(i.vi[0]);
    return t;
}

void FresnelSSE(const FresnelSSEInput* i,float *o)
{
    _mm_store_ss(o,FresnelSSE(i->n1n2,i->costheta));
}

FresnelInput stim()
{
    FresnelInput i;
    i.n1 = 1.0+uni01(rng);
    i.n2 = 1.0+uni01(rng);

    double vi_d[2];

    double theta,&costheta=vi_d[0],&sintheta=vi_d[1],sintheta_t;
    do {
        theta = 3.141592654/2*uni01(rng);
        costheta = cos(theta);
        sintheta = sin(theta);
        sintheta_t = i.n1*sintheta/i.n2;
    }
    while (sintheta_t>1.0);     // ensure no TIR


    double vt_d[2] = { sqrt(1-sintheta_t*sintheta_t), sintheta_t };

    i.vi = UnitVector<2,double>(vi_d);
    i.vt = UnitVector<2,double>(vt_d);

    return i;
}

ostream& operator<<(ostream& os,const FresnelInput& i)
{
    return os << "vi=" << i.vi << " vt=" << i.vt;
}

ostream& operator<<(ostream& os,const RefractInput& i)
{
    return os << "n1=" << i.n1 << " n2=" << i.n2 << " vi=" << i.vi;
}

int main(int argc,char **argv)
{
    TimeBench<FresnelInput,double,Tolerance<double> > fresnelbench(FresnelBase,10000000,Tolerance<double>(0.0,1e-4));

    fresnelbench.makeStimulus(stim);

    Result r;

    r=fresnelbench.run(FresnelBase);
    cout << "Ran in " << r.t_user*1e6 << " usec with " << r.Nerr << " errors" << endl;

    r=fresnelbench.run(FresnelSSE,FresnelSSEMap(),Identity<float,double>());
    cout << "Ran in " << r.t_user*1e6 << " usec with " << r.Nerr << " errors" << endl;



    TimeBench<RefractInput,UnitVector<2,double>,VectorCompare<2,double> > refractbench
        (RefractBase,10000000,VectorCompare<2,double>(1e-3));
    refractbench.makeStimulus(RefractStim);

    cout << "Refraction - reference case" << endl;
    r = refractbench.run(RefractBase);
    cout << "Ran in " << r.t_user*1e6 << " usec with " << r.Nerr << " errors" << endl;

    cout << "Refraction - test code" << endl;
    r = refractbench.run(RefractSSEWrap,RefractSSEMap(),RefractSSEOMap());
    cout << "Ran in " << r.t_user*1e6 << " usec with " << r.Nerr << " errors" << endl;
}
