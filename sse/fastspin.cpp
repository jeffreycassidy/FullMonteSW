#include <pmmintrin.h>
#include "ssehelp.cpp"
#include <boost/timer/timer.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

#include <boost/math/constants/constants.hpp>

#include "../newgeom.hpp"
#include "../graph.hpp"
#include "../source.hpp"
#include "../sse.hpp"

#include "timebench.hpp"

using namespace boost::timer;
using namespace std;

boost::random::mt19937 rng;
boost::random::uniform_01<double> uni01;

typedef struct {
    float costheta,sintheta,cosphi,sinphi;
} AngleStruct;

typedef union {
    AngleStruct f;
    __m128 v;
} ScatterAngle;

class SuffixPrinterPlaceholder {
    int ndec,offset;
    static const char* suffix;

    public:
    SuffixPrinterPlaceholder(int ndec_,int offset_) : ndec(ndec_),offset(offset_){};

    string operator()(double f) const;
};

class SuffixPrinter : public SuffixPrinterPlaceholder {
    ostream& os;
    public:

    SuffixPrinter(ostream& os_,const SuffixPrinterPlaceholder& sp_) : SuffixPrinterPlaceholder(sp_), os(os_){};

    friend ostream& operator<<(const SuffixPrinter&,double f);
};

SuffixPrinterPlaceholder floatSuffix(int ndec,int n_offset=0)
{
    return SuffixPrinterPlaceholder(ndec,n_offset);
}

ostream& operator<<(const SuffixPrinter& sp_,double f);
SuffixPrinter operator<<(ostream& os,const SuffixPrinterPlaceholder& sp_);

template<class UnaryFunction,class InputIterator,class OutputIterator> boost::timer::cpu_times timedMap(UnaryFunction& f,InputIterator begin,const InputIterator& end,OutputIterator o,string s="");

// Runs a timed fold
template<class BinaryFunction,class State,class InputIterator> pair<boost::timer::cpu_times,State> timedFold(const BinaryFunction& f,const State& st0,InputIterator begin,const InputIterator& end,string s="")
{//<BinaryFunction,State,InputIterator>
    State st=st0;
    boost::timer::cpu_timer t;
    if (s.length() > 0)
        cout << "Running fold: " << s << endl;
    cacheWipe(64000000);
    t.start();
    for(; begin != end; ++begin)
        st = f(st,*begin);
    t.stop();
    if (s.length() > 0)
        cout << "  Took " << floatSuffix(3,-9) << t.elapsed().wall << 's' << endl;
    return make_pair(t.elapsed(),st);
}

// Runs a timed fold
template<class BinaryFunction,class State,class InputIterator> pair<boost::timer::cpu_times,vector<State> > timedFold2(const BinaryFunction& f,const State& st0,InputIterator begin,const InputIterator& end,string s="")
{//<BinaryFunction,State,InputIterator>
    State a(st0),b(st0);
    boost::timer::cpu_timer t;
    if (s.length() > 0)
        cout << "Running fold stride 2: " << s << endl;
    cacheWipe(64000000);
    t.start();
    while(begin != end)
    {
        a = f(a,*(begin++));
        b = f(b,*(begin++));
    }
    t.stop();
    if (s.length() > 0)
        cout << "  Took " << floatSuffix(3,-9) << t.elapsed().wall << 's' << endl;

    vector<State> st(2);
    st[0]=a;
    st[1]=b;
    return make_pair(t.elapsed(),st);
}


// Runs a timed map
template<class UnaryFunction,class InputIterator,class OutputIterator> boost::timer::cpu_times timedMap(const UnaryFunction& f,InputIterator begin,const InputIterator& end,OutputIterator o,string s="")
{
    boost::timer::cpu_timer t;
    if (s.length() > 0)
        cout << "Running map: " << s << endl;
    cacheWipe(64000000);
    t.start();
    for(; begin != end; ++begin,++o)
        *o = f(*begin);
    t.stop();
    if (s.length() > 0)
        cout << "  Took " << floatSuffix(3,-9) << t.elapsed().wall << 's' << endl;
    return t.elapsed();
}

// Runs a timed zipWith (binary function)
template<class BinaryFunction,class InputIterator1,class InputIterator2,class OutputIterator> boost::timer::cpu_times timedZipWith(const BinaryFunction& f,InputIterator1 begin1,const InputIterator1& end1,InputIterator2 begin2,OutputIterator o,string s="")
{
    boost::timer::cpu_timer t;
    if (s.length() > 0)
        cout << "Running binary map: " << s << endl;
    cacheWipe(64000000);
    t.start();
    for(; begin1 != end1; ++begin1,++begin2,++o)
        *o = f(*begin1,*begin2);
    t.stop();
    if (s.length() > 0)
        cout << "  Took " << floatSuffix(3,-9) << t.elapsed().wall << 's' << endl;
    return t.elapsed();
}

const char suffixes[] = "fpnum kMGT";
const char* SuffixPrinterPlaceholder::suffix = suffixes+5;

string SuffixPrinterPlaceholder::operator()(double f) const
{
    int i;
    f *= pow10(offset);
    for(i=0;    fabs(f)<1.0;       f*=1000.0,--i);
    for(;       fabs(f)>1000.0;    f/=1000.0,++i);
    stringstream ss;
    ss << fixed << setprecision(ndec) << f << suffix[i];
    return ss.str();
}

SuffixPrinter operator<<(ostream& os,const SuffixPrinterPlaceholder& sp_)
{
    return SuffixPrinter(os,sp_);
}

ostream& operator<<(const SuffixPrinter& sp_,double f)
{
    return sp_.os << sp_(f);
}

bool checkPacket(Packet p)
{
    SSEReg_t mask;
    bool flag=false;

    mask.m128_si = _mm_set1_epi32(0x7fffffff);
    __m128 r0=p.d,r1=p.a,r2=p.b,r3=p.b;
    _MM_TRANSPOSE4_PS(r0,r1,r2,r3);                     // transpose so all x/y/z elements are together

    __m128 norm2 = _mm_mul_ps(r0,r0);                   // compute x^2, y^2, z^2 for each vector
    norm2 = _mm_add_ps(norm2,_mm_mul_ps(r1,r1));
    norm2 = _mm_add_ps(norm2,_mm_mul_ps(r2,r2));

    __m128 norm = _mm_sqrt_ps(norm2);

    __m128 err = _mm_sub_ps(norm,_mm_set1_ps(1.0)); // sqrt(x^2+y^2+z^2)-1

    if (_mm_movemask_ps(_mm_cmpge_ps(_mm_and_ps(err,mask.m128_f),_mm_set1_ps(1e-5))))   // compare abs to eps
    {
        cerr << "Error: packet direction fails unit test with norms " << norm << endl;
        flag=true;
    }

    __m128 dot_da = _mm_mul_ps(p.d,p.a);        // 0 dz*az dy*ay dx*ax
    __m128 dot_db = _mm_mul_ps(p.d,p.b);        // 0 dz*bz dy*by dx*bx
    __m128 dot_ab = _mm_mul_ps(p.a,p.b);        // 0 az*bz ay*by ax*bx

    // unpacklo_ps(a,b) => b1,a1,b0,a0

    // movehl_ps(a,b) => a3,a2,b3,b2

    // movelh_ps(a,b) => b1,b0,a1,a0

    __m128 dots = _mm_hadd_ps(_mm_hadd_ps(_mm_setzero_ps(),dot_ab),_mm_hadd_ps(dot_db,dot_da));

    if (_mm_movemask_ps(_mm_cmpge_ps(_mm_and_ps(mask.m128_f,dots),_mm_set1_ps(1e-5))))
    {
        cerr << "Error: packet fails orthogonality test with dots (0,ab,db,ab): " << dots << endl;
        flag = true;
    }

    return flag;
}


class ToleranceSSE4f {
    __m128 eps;
    public:
    ToleranceSSE4f(double eps_) : eps(_mm_set_ps(0,0,0,eps_*eps_)){}

    bool operator()(__m128 a,__m128 b) const {
        __m128 diff = _mm_sub_ps(a,b);
        return _mm_movemask_ps(_mm_cmplt_ss(dot(diff,diff),eps));
    }
};
/*
__m128 normalize_approx(__m128 x)
{
    __m128 xx=_mm_mul_ps(x,x);
    __m128 sum1 = _mm_add_ps(xx,_mm_shuffle_ps(xx,xx,_MM_SHUFFLE(2,3,0,1)));
    __m128 sum2 = _mm_add_ps(sum1,_mm_shuffle_ps(sum1,sum1,_MM_SHUFFLE(0,0,2,2)));
    __m128 l = _mm_rsqrt_ps(sum2);

    return _mm_mul_ps(x,l);
}*/
/*
__m128 vecmin(__m128 x)
{
    __m128 min1 = _mm_min_ps(x,_mm_shuffle_ps(x,x,_MM_SHUFFLE(2,3,1,0)));
    __m128 min2 = _mm_min_ps(x,_mm_shuffle_ps(min1,min1,_MM_SHUFFLE(0,0,2,2)));
    return min2;
}

inline __m128 ssemin4f(__m128 v)
{
    __m128 min1 = _mm_min_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,3,0,1)));
    return  _mm_min_ps(min1,_mm_shuffle_ps(min1,min1,_MM_SHUFFLE(0,0,2,2)));
}

inline __m128 ssemax4f(__m128 v)
{
    __m128 max1 = _mm_max_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,3,0,1)));
    return  _mm_max_ps(max1,_mm_shuffle_ps(max1,max1,_MM_SHUFFLE(0,0,2,2)));
}*/


// original, unmodified;
//template<class T>UnitVector<3,T> Scatter(double g,const UnitVector<3,T>& d0,double rnd0=rand01(),double rnd1=rand01())

//__m128 Scatter(double g,const UnitVector<3,T>& d0,double rnd0=rand01(),double rnd1=rand01())

SSEReg_t ScatterOld(SSEReg_t pkt,ScatterAngle sa)
{
    const float dx=pkt.f[0],dy=pkt.f[1],dz=pkt.f[2];

    float costheta = sa.f.costheta;
    float sintheta = sa.f.sintheta;

    const float cosp=sa.f.cosphi;
    const float sinp=sa.f.sinphi;

    SSEReg_t d=pkt;
    d.f[0] *= costheta;
    d.f[1] *= costheta;
    d.f[2] *= costheta;

    // find largest Cartesian component to use for cross product
    unsigned min_i=0;
    float min_u=abs(pkt.f[0]),tmp;
    if ((tmp=abs(pkt.f[1])) < min_u)
        min_i=1,min_u=tmp;
    if ((tmp=abs(pkt.f[2])) < min_u)
        min_i=2,min_u=tmp;

    float k=1/sqrt(1-min_u*min_u);

    switch(min_i){
        case 0: // use X component, n1 = (d) cross (i); n2 = (d) cross (n1)
        d.f[0]+=k*sintheta*(0 - sinp*(dy*dy+dz*dz));
        d.f[1]+=k*sintheta*(cosp*dz + sinp*dx*dy);
        d.f[2]+=k*sintheta*(-cosp*dy + sinp*dx*dz); 
        break;
        case 1: // use Y component
        d.f[0]+=k*sintheta*(-cosp*dz + sinp*dx*dy);
        d.f[1]+=k*sintheta*(0 - sinp*(dx*dx+dz*dz));
        d.f[2]+=k*sintheta*(cosp*dx + sinp*dy*dz);
        break;
        case 2: // use Z component
        d.f[0]+=k*sintheta*(cosp*dy + sinp*dx*dz);
        d.f[1]+=k*sintheta*(-cosp*dx + sinp*dy*dz);
        d.f[2]+=k*sintheta*(0 - sinp*(dx*dx+dy*dy)); 
        break;
        default:
        assert(0);
        break;
    }

    // includes assertion check that it actually is unit ('true' argument below does not scale by length)
//    UnitVector<3,float> d_v(d,false);          // TODO: See if must be false

    return d;
}

//__m128 Scatter(double g,const UnitVector<3,T>& d0,double rnd0=rand01(),double rnd1=rand01())
SSEReg_t ScatterSSE(SSEReg_t pkt,ScatterAngle sa)
{
    SSEReg_t d = pkt;
    const float& dx=pkt.f[0], &dy=pkt.f[1], &dz=pkt.f[2];

    // find largest Cartesian component to use for cross product
    unsigned min_i=0;
    float min_u=fabs(pkt.f[0]),tmp;
    if ((tmp=fabs(pkt.f[1])) < min_u)
        min_i=1,min_u=tmp;
    if ((tmp=fabs(pkt.f[2])) < min_u)
        min_i=2,min_u=tmp;

    float k=sa.f.sintheta/sqrtf(1-min_u*min_u);

    __m128 dd;

    switch(min_i){
        case 0: // use X component, n1 = (d) cross (i); n2 = (d) cross (n1)
        dd = _mm_setr_ps(-sa.f.sinphi*(dy*dy+dz*dz), sa.f.cosphi*dz + sa.f.sinphi*dx*dy, -sa.f.cosphi*dy + sa.f.sinphi*dx*dz, 0); 
        break;
        case 1: // use Y component
        dd = _mm_setr_ps(-sa.f.cosphi*dz + sa.f.sinphi*dx*dy, -sa.f.sinphi*(dx*dx+dz*dz), sa.f.cosphi*dx + sa.f.sinphi*dy*dz, 0);
        break;
        case 2: // use Z component
        dd = _mm_setr_ps((sa.f.cosphi*dy + sa.f.sinphi*dx*dz), (-sa.f.cosphi*dx + sa.f.sinphi*dy*dz), -sa.f.sinphi*(dx*dx+dy*dy), 0); 
        break;
    }

    SSEReg_t o;

    o.m128_f = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(k),dd),_mm_mul_ps(_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(0,0,0,0)),pkt.m128_f));
//    o.m128_f = normalize(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(k),dd),_mm_mul_ps(_mm_set1_ps(costheta),d0)));

    return o;
}

inline SSEReg_t TIMOSScatter(SSEReg_t in,ScatterAngle sa)
{
  double cost=sa.f.costheta, sint=sa.f.sintheta, cosp=sa.f.cosphi, sinp=sa.f.sinphi;

    SSEReg_t o;

    double ux=in.f[0]; 
    double uy=in.f[1]; 
    double uz=in.f[2];

    double temp1,temp,temp2;
    
    if(fabs(uz)<=0.99999){
      temp1 = sqrt(1.0 - uz*uz);
      temp  = sint/temp1;
      temp2 = uz*cosp;
      o.f[0] = (ux*temp2 - uy*sinp)*temp + ux*cost;
      o.f[1] = (uy*temp2 + ux*sinp)*temp + uy*cost;
      o.f[2] = -sint*cosp*temp1 + uz*cost;      
    }else{
      o.f[0] = sint * cosp;
      o.f[1] = sint * sinp;
      o.f[2] = ((uz>0) ?  cost : -cost);
    }

  return o;
}

inline SSEReg_t TIMOSScatterF(SSEReg_t in,ScatterAngle sa)
{
  float cost=sa.f.costheta, sint=sa.f.sintheta, cosp=sa.f.cosphi, sinp=sa.f.sinphi;

    SSEReg_t o;

    float ux=in.f[0]; 
    float uy=in.f[1]; 
    float uz=in.f[2];

    float temp1,temp,temp2;
    
    if(fabs(uz)<=0.99999){
      temp1 = sqrt(1.0 - uz*uz);
      temp  = sint/temp1;
      temp2 = uz*cosp;
      o.f[0] = (ux*temp2 - uy*sinp)*temp + ux*cost;
      o.f[1] = (uy*temp2 + ux*sinp)*temp + uy*cost;
      o.f[2] = -sint*cosp*temp1 + uz*cost;      
    }else{
      o.f[0] = sint * cosp;
      o.f[1] = sint * sinp;
      o.f[2] = ((uz>0) ?  cost : -cost);
    }

  return o;
}

inline SSEReg_t TIMOSScatterV(SSEReg_t in,ScatterAngle sa)
{
  float cost=sa.f.costheta, sint=sa.f.sintheta, cosp=sa.f.cosphi, sinp=sa.f.sinphi;

    SSEReg_t o;

    __m128 one = _mm_set_ss(1.0);

    __m128 uz = _mm_shuffle_ps(in.m128_f,in.m128_f,_MM_SHUFFLE(2,2,2,2));

    if (_mm_movemask_ps(_mm_cmpge_ps(_mm_set_ss(0.999999),uz))){
//    if(fabs(uz)<=0.99999){
      o.m128_f = _mm_mul_ps(in.m128_f,_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(0,0,0,0)));       // d*costheta

      __m128 uu = _mm_mul_ps(in.m128_f,_mm_shuffle_ps(in.m128_f,in.m128_f,_MM_SHUFFLE(2,2,2,2)));    // [0 uz uy ux] * [uz uz uz uz]

      __m128 one_minus_dz2=_mm_sub_ps(one,_mm_shuffle_ps(uu,uu,_MM_SHUFFLE(2,2,2,2)));
      __m128 k = _mm_rsqrt_ps(one_minus_dz2);

      __m128 us =       _mm_shuffle_ps(uu,in.m128_f,_MM_SHUFFLE(0,1,1,0));   //  ux      uy      uy*uz   ux*uz
      __m128 trigs =    _mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(3,3,2,2)); //  sinp    sinp    cosp    cosp 
      __m128 t0 = _mm_mul_ps(us,trigs);

      __m128 t1 = _mm_addsub_ps(
        _mm_movelh_ps(t0,_mm_setzero_ps()),                                                 // 0        0       uy*uz*cosp  ux*uz*cosp 
        _mm_movelh_ps(us,_mm_mul_ss(one_minus_dz2,_mm_move_ss(_mm_setzero_ps(),trigs))));   // 0        cosp    ux*sinp     uy*sinp

      o.m128_f = _mm_add_ps(o.m128_f,_mm_mul_ps(k,t1));     // calc final output

//      b = d x k

    // d cross k

//      o = _mm_add_ps(
//      o.f[0] = (ux*temp2 - uy*sinp)*temp;         // ux*uz*cosp*sint/temp1 - uy*sinp*sint/temp1
//      o.f[1] = (uy*temp2 + ux*sinp)*temp;         // uy*uz*cosp*sint/temp1 + ux*sinp*sint/temp1
//      o.f[2] = -sint*cosp*temp1;                  // -sint*cosp*temp1
    }
    else if (_mm_movemask_ps(uz)){
      o.f[0] = sint * cosp;
      o.f[1] = sint * sinp;
      o.f[2] = -cost;
    }
    else {
        o.f[0] = sint*cosp;
        o.f[1] = sint*sinp; 
        o.f[2] = cost;
    }

  return o;
}


inline SSEReg_t spinsse(SSEReg_t in,ScatterAngle sa)
{
    SSEReg_t o;
    __m128 d0=in.m128_f;

    // find smallest Cartesian component to use for cross product
/*    float f[4];
    _mm_store_ps(f,in->d);
    unsigned min_i=0;
    double min_u=abs(f[0]),tmp;
    if ((tmp=abs(f[1])) < min_u)
        min_i=1,min_u=tmp;
    if ((tmp=abs(f[2])) < min_u)
        min_i=2,min_u=tmp;*/

//    __m128 trig = _mm_movelh_ps(in->_00_sinp_cosp.vf,_mm_sub_ps(_mm_setzero_ps(),in->_00_sinp_cosp.vf));

    __m128 trig = sa.v;

//    __m128 trig = _mm_set_ps(-sin(in->phi),-cos(in->phi),sin(in->phi),cos(in->phi));

    __m128 ones = _mm_set_ss(1.0);
//    __m128 cost = in->_000_cost.vf;

//    __m128 k = _mm_mul_ps(sqrt((1-in->costheta*in->costheta)/(1-min_u*min_u))),d0);
    __m128 dz = _mm_shuffle_ps(d0,d0,_MM_SHUFFLE(2,2,2,2));
    __m128 dz2 = _mm_mul_ps(dz,dz);     // k = sin(theta)/sqrt(1-dz^2)
    __m128 k = _mm_div_ps(d0,_mm_sqrt_ps(_mm_sub_ps(ones,dz2)));

    __m128 dshuf=d0;

    if (_mm_movemask_ps(_mm_cmpgt_ps(_mm_and_ps(_mm_set1_ps(0x7fffffff),dz),_mm_set1_ps(1-1e-4))))
        dshuf=_mm_shuffle_ps(d0,d0,_MM_SHUFFLE(3,0,2,1));

    // ax = by cz - bz cy
    // ay = bz cx - bx cz
    // az = bx cy - by cx

    __m128 cbbc = _mm_shuffle_ps(dshuf,dshuf,_MM_SHUFFLE(2,1,1,2));
    __m128 cbaa = _mm_shuffle_ps(dshuf,dshuf,_MM_SHUFFLE(0,0,1,2));
    __m128 cc_bb_ab_ac = _mm_mul_ps(cbbc,cbaa);

    __m128 nsp_nsp_sp_sp = _mm_shuffle_ps(trig,trig,_MM_SHUFFLE(3,3,1,1));
    __m128 t = _mm_mul_ps(nsp_nsp_sp_sp,cc_bb_ab_ac);

    // get b*cos(phi), c*cos(phi)
    __m128 xxcb = _mm_shuffle_ps(dshuf,dshuf,_MM_SHUFFLE(0,0,2,1));
    __m128 xx_cphi_ncphi = _mm_shuffle_ps(trig,trig,_MM_SHUFFLE(0,0,2,0));
    __m128 ccp_bcp = _mm_mul_ps(xxcb,xx_cphi_ncphi);

    __m128 loadd = _mm_unpacklo_ps(ccp_bcp,t);
    __m128 hiadd = _mm_movehl_ps(_mm_setzero_ps(),t);

    _mm_movelh_ps(hiadd,loadd);

    __m128 bc0a = _mm_hadd_ps(hiadd,loadd);

    // results should now be <b c 0 a> - just need shuffling into place

    __m128 dd = _mm_shuffle_ps(bc0a,bc0a,_MM_SHUFFLE(1,2,3,0));

    // d = d0 * costheta + sintheta/sqrt(1-min_u*min_u) * n
    __m128 d = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(0,0,0,0)),d0),_mm_mul_ps(k,dd));

    o.m128_f = normalize(d);

    return o;
}

// Unit vector representation in __m128 is [0 0 sin(theta) cos(theta)] corresponding with
// f[0] = cos(theta) f[1] = sin(theta) in memory
//
// sa is packed [sin(phi) cos(phi) sin(theta) cos(theta)]

inline Packet matspin_new(Packet pkt,ScatterAngle sa)
{
    Packet res;

    // colums of matrix M
    __m128 M0,M1,M2;

    // _000_cost = 0 0 0 cost
/*	__m128 _00_cost_sint = _mm_sqrt_ss(
		_mm_sub_ps(
			_mm_unpacklo_ps(_mm_set_ss(1.0),_000_cost),     // 0    0   cost    1.0
			_mm_mul_ss(_000_cost,_000_cost)));              // 0    0   0       cost^2*/

	__m128 zero = _mm_setzero_ps();
	__m128 strig = _mm_addsub_ps(zero,sa.v);	// (-sin phi) (cos phi) (-sin theta) (cos theta)

	__m128 prods = _mm_mul_ps(
        strig,                                              // -sinp cosp -sint cost
        _mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));  // -sint cost cosp  -sinp
    // prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

	__m128 _0_sp_0_cp = _mm_unpackhi_ps(sa.v,zero);  // 0 sinp 0 cosp

    // The following 3 defs are verified to match M0..M2 in comments above
	M0 = _mm_movelh_ps(sa.v,zero);                                  // 0 0 sint cost
	M1 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,2,2,1));     // 0 sinp cost*cosp -sint*cosp
	M2 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,0,0,3));     // 0 cosp -cost*sinp sint*sinp

    // d = cos(theta)*d0 - sin(theta)*cos(phi)*a0 + sin(theta)*sin(phi)*b0
    res.d = _mm_mul_ps(pkt.d,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(0,0,0,0)));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(pkt.a,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(0,0,0,0))));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(pkt.b,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(0,0,0,0))));

    // a = sin(theta)*d0 +cos(theta)*cos(phi)*a0 - cos(theta)*sin(phi)*b0
    res.a = _mm_mul_ps(pkt.d,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(1,1,1,1)));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(pkt.a,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(1,1,1,1))));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(pkt.b,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(1,1,1,1))));

    // b = sin(phi)*a0 + cos(phi)*b0
    res.b = _mm_mul_ps(pkt.a,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2)));
    res.b = _mm_add_ps(res.b,_mm_mul_ps(pkt.b,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(2,2,2,2))));

    return res;
}

inline Packet matspin_new2(Packet pkt,ScatterAngle sa)
{
    Packet res;

    // From Agner Fog on Sandy Bridge
    // shuffle: lat 1, throughput 1
    // unpackl/h: lat 1, throughput 1
    // ie shuffle/unpack are equivalent

    // c = _mm_shuffle_ps(a,b,_MM_SHUFFLE(l,k,j,i));
    // c[0] = a[i]
    // c[1] = a[j]
    // c[2] = b[k]
    // c[3] = b[l]

    // c = _mm_movelh_ps(a,b)  == _mm_shuffle_ps(a,b,_MM_SHUFFLE(1,0,1,0))
    // c[0] = a[0]
    // c[1] = a[1]
    // c[2] = b[0]
    // c[3] = b[1]

    // c = _mm_movehl_ps(a,b) = _mm_shuffle_ps(a,b,_MM_SHUFFLE())
    // =

	__m128 zero = _mm_setzero_ps();
	__m128 strig = _mm_addsub_ps(zero,sa.v);	// (-sin phi) (cos phi) (-sin theta) (cos theta)

	__m128 prods = _mm_mul_ps(
        strig,                                              // -sinp cosp -sint cost
        _mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));  // -sint cost cosp  -sinp

    // prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

    // sa = (sinphi) (cosphi) (sintheta) (costheta)

//	__m128 _0_sp_0_cp = _mm_unpackhi_ps(sa.v,zero);  // 0 sinp 0 cosp

    // The following 3 defs are verified to match M0..M2 in comments above
//	M0 = _mm_movelh_ps(sa.v,zero);                                  // 0 0 sint cost
//	M1 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,2,2,1));     // 0 sinp cost*cosp -sint*cosp
//	M2 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,0,0,3));     // 0 cosp -cost*sinp sint*sinp

    // d = cos(theta)*d0 - sin(theta)*cos(phi)*a0 + sin(theta)*sin(phi)*b0
    res.d = _mm_mul_ps(pkt.d,_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(0,0,0,0)));
    res.a = _mm_mul_ps(pkt.d,_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(1,1,1,1)));
    res.b = _mm_mul_ps(pkt.a,_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(3,3,3,3)));

    res.d = _mm_add_ps(res.d,_mm_mul_ps(pkt.a,_mm_shuffle_ps(prods,prods,_MM_SHUFFLE(1,1,1,1))));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(pkt.a,_mm_shuffle_ps(prods,prods,_MM_SHUFFLE(2,2,2,2))));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(pkt.b,_mm_shuffle_ps(prods,prods,_MM_SHUFFLE(3,3,3,3))));

    // a = sin(theta)*d0 +cos(theta)*cos(phi)*a0 - cos(theta)*sin(phi)*b0
    res.a = _mm_add_ps(res.a,_mm_mul_ps(pkt.b,_mm_shuffle_ps(prods,prods,_MM_SHUFFLE(0,0,0,0))));

    // b = sin(phi)*a0 + cos(phi)*b0
    res.b = _mm_add_ps(res.b,_mm_mul_ps(pkt.b,_mm_shuffle_ps(sa.v,sa.v,_MM_SHUFFLE(2,2,2,2))));

    return res;
}

void stimGen(unsigned N,vector<ScatterAngle>& v_sa,vector<Packet>& v_pkt,vector<SSEReg_t>& v_d)
{
    v_sa.clear();
    v_pkt.clear();
    v_d.clear();

    v_sa.resize(N);
    v_pkt.resize(N);
    v_d.resize(N);

    for(unsigned i=0;i<N;++i){
        ScatterAngle sa;
        Packet pkt;
        unsigned min_i;
        double min_d,tmp;
        double p[3] = { uni01(rng)-0.5, uni01(rng)-0.5, uni01(rng)-0.5 }; 

        UnitVector<3,double> dv(p);
    
        pkt.d = dv;
    
        for(unsigned j=0;j<3;++j)
            p[j] = dv[j];
    
        // find minimum component
        min_i=0;
        min_d=fabs(p[0]);
        if ((tmp=fabs(p[1]))<min_d)
            min_i=1,min_d=tmp;
        if ((tmp=fabs(p[2]))<min_d)
            min_i=2,min_d=tmp;
    
        // set up a,b vectors similar to existing Scatter routine
        //  a = d (cross) b
        //  b = d (cross) a
    
        tmp = sqrt(1-min_d*min_d);
    
        switch(min_i){
            case 0:
            pkt.b = _mm_setr_ps(0.0,p[2]/tmp,-p[1]/tmp,0.0);
            pkt.a = -_mm_setr_ps((-p[2]*p[2]-p[1]*p[1])/tmp,p[0]*p[1]/tmp,p[0]*p[2]/tmp,0.0);
            break;
            case 1:
            pkt.b = _mm_setr_ps(-p[2]/tmp,0.0,p[0]/tmp,0.0);
            pkt.a = -_mm_setr_ps(p[0]*p[1]/tmp,-(p[0]*p[0]+p[2]*p[2])/tmp,p[1]*p[2]/tmp,0.0);
            break;
            case 2:
            pkt.a = _mm_setr_ps(p[1]/tmp,-p[0]/tmp,0.0,0.0);
            pkt.b = _mm_setr_ps(p[0]*p[2]/tmp,p[1]*p[2]/tmp,-(p[0]*p[0]+p[1]*p[1])/tmp,0.0);
            break;
        }
    
        float phi=2.0*3.141592654*uni01(rng);
        sincosf(phi,&sa.f.sinphi,&sa.f.cosphi);
    
        float theta=2.0*3.141592654*uni01(rng);
        sincosf(theta,&sa.f.sintheta,&sa.f.costheta);

        v_sa[i]  = sa;
        v_pkt[i] = pkt;
        v_d[i].m128_f   = pkt.d;
    }
}

float scalarDot(__m128 a,__m128 b)
{
    float f;
    _mm_store_ss(&f,_mm_dp_ps(a,b,0xff));
    return f;
}

unsigned addOne(unsigned a)
{
    return a+1;
}

unsigned add(unsigned a,unsigned b){ return a+b; }

Packet initpkt()
{
    Packet t;
    __m128 one = _mm_set1_ps(1.0);
    t.d = _mm_shuffle_ps(one,one,_MM_SHUFFLE(0,0,0,1));
    t.a = _mm_shuffle_ps(one,one,_MM_SHUFFLE(0,0,1,0));
    t.b = _mm_shuffle_ps(one,one,_MM_SHUFFLE(0,1,0,0));
    return t;
}

int main(int argc,char **argv)
{
    const unsigned N=10000000;

//    TimeBench<input,__m128,ToleranceSSE4f> bench(Scatter,N,ToleranceSSE4f(1e-4));

//    bench.makeStimulus(stimGen);

    unsigned *i = new unsigned[N];
    unsigned *o = new unsigned[N];

    for(unsigned j=0;j<N;++j)
        i[j] = rand();

/*    cout << "Running SSE test code: " << endl;
    r = bench.run(spinsse);
    cout << "Took " << r.t_user*1e3 << "us; Found " << r.Nerr << " discrepancies in test" << endl;
*/

//    pair<boost::timer::cpu_times,unsigned> p = timedFold(add,0U,test,test+10,"summing first 10 integers");

    vector<ScatterAngle> v_sa;
    vector<Packet> v_pkt;
    vector<SSEReg_t> v_d;

    stimGen(N,v_sa,v_pkt,v_d);

    SSEReg_t tmp;
    tmp.m128_f = _mm_set1_ps(1.0);

    pair<boost::timer::cpu_times,SSEReg_t> p_old = timedFold(spinsse,tmp,v_sa.begin(),v_sa.end(),"Old spin calc");

    pair<boost::timer::cpu_times,SSEReg_t> p_ref  = timedFold(ScatterOld,tmp,v_sa.begin(),v_sa.end(),"Reference spin calc");
    pair<boost::timer::cpu_times,SSEReg_t> p_refs = timedFold(ScatterSSE,tmp,v_sa.begin(),v_sa.end(),"SSE-enhanced reference spin calc");

    pair<boost::timer::cpu_times,SSEReg_t> p_tim = timedFold(TIMOSScatter,tmp,v_sa.begin(),v_sa.end(),"TIM-OS scatter");
    pair<boost::timer::cpu_times,SSEReg_t> p_timf = timedFold(TIMOSScatterF,tmp,v_sa.begin(),v_sa.end(),"TIM-OS scatter (float)");
    pair<boost::timer::cpu_times,SSEReg_t> p_timv = timedFold(TIMOSScatterV,tmp,v_sa.begin(),v_sa.end(),"TIM-OS scatter (m128)");

    pair<boost::timer::cpu_times,Packet> p_matrix2 = timedFold(matspin_new2,initpkt(),v_sa.begin(),v_sa.end(),"Matrix spin-update");
    pair<boost::timer::cpu_times,Packet> p_matrix = timedFold(matspin_new,initpkt(),v_sa.begin(),v_sa.end(),"Matrix spin");

    pair<boost::timer::cpu_times,vector<SSEReg_t> > p_ref_2  = timedFold2(ScatterOld,tmp,v_sa.begin(),v_sa.end(),"Reference spin calc");
    pair<boost::timer::cpu_times,vector<SSEReg_t> > p_refs_2 = timedFold2(ScatterSSE,tmp,v_sa.begin(),v_sa.end(),"SSE-enhanced reference spin calc");
    pair<boost::timer::cpu_times,vector<Packet> > p_matrix2_2 = timedFold2(matspin_new2,initpkt(),v_sa.begin(),v_sa.end(),"Matrix spin-update");
    pair<boost::timer::cpu_times,vector<Packet> > p_matrix_2  = timedFold2(matspin_new,initpkt(),v_sa.begin(),v_sa.end(),"Matrix spin");

    pair<boost::timer::cpu_times,vector<SSEReg_t> > p_timv_2  = timedFold2(TIMOSScatterV,tmp,v_sa.begin(),v_sa.end(),"Matrix spin");

    {
        unsigned err_pkt=0;
        unsigned err_dot=0;
        vector<Packet> o(N);
        transform(v_pkt.begin(),v_pkt.end(),v_sa.begin(),o.begin(),matspin_new);

        for(unsigned i=0;i<N;++i)
        {
            err_pkt += checkPacket(o[i]);
            err_dot += fabs(scalarDot(o[i].d,v_pkt[i].d)-v_sa[i].f.costheta)>1e-5;
        }
        cout << "Checking matspin_new2: " << err_pkt << " orthogonality errors, " << err_dot << " dot errors of " << N << endl;
    }

//    r = bench.run(Scatter);
//    cout << "Took " << floatSuffix(2,-9) << r.t_user << "s; Found " << r.Nerr << " discrepancies in test" << endl;

/*    cout << "Running SSE-enhanced reference code: " << endl;
    r = bench.run(ScatterSSE);
    cout << "Took " << r.t_user*1e3 << "us; Found " << r.Nerr << " discrepancies in test" << endl;

    cout << "Running SSE matrix code: " << endl;
    r = bench.run(matspin);
    cout << "Took " << r.t_user*1e3 << "us; Found " << r.Nerr << " discrepancies in test" << endl;

//    cout << "Running SSE matrix code (new ver): " << endl;
//    r = bench.run(matspin_new);
//    cout << "Took " << r.t_user*1e3 << "us" << endl;

    cout << "Running iterative (reg-packed) matrix spin" << endl;
    {
        boost::timer::cpu_timer t;
        Packet pkt;

        t.start();

//        for(unsigned i=0;i<N;++i)
//            pkt = matspin(pkt,*i);
        t.stop();
        boost::timer::cpu_times r = t.elapsed();
        cout << "Took " << r.user*1e6 << "us" << endl;
    }*/
}

