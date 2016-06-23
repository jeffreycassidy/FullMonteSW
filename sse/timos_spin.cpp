#include <math.h>

#include <boost/timer/timer.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/math/constants/constants.hpp>

#include <emmintrin.h>
#include <smmintrin.h>
#include <pmmintrin.h>

#include "../sse.hpp"

#define G_COS_0_D 1.0-1e-14

boost::random::mt19937 rng;
boost::random::uniform_01<double> uni01;

using namespace std;

void cacheWipe()
{
    uint8_t* p = new uint8_t[10000000];
    for(unsigned i=0;i<10000000;++i)
        p[i]=i;
    delete[] p;
}

template<class T> class Angle {
    public:
    T cosp,sinp,cost,sint;
};

template<> class Angle<__m128> {
    public:
    __m128 trig;
    Angle(){}
    Angle(const Angle<double>& a_) : trig(_mm_set_ps(a_.cosp,a_.sinp,a_.cost,a_.sint)){}
};

template<class T>class Packet {
    public:
    T ux,uy,uz;
};

template<class T>inline Packet<T> timos_spin(T cosp,T sinp,T cost,T sint,Packet<T> p)
{
    Packet<T> Photon;

    T temp1,temp,temp2;

    if(fabs(p.uz)<=G_COS_0_D){
      temp1 = sqrt(1.0 - p.uz*p.uz);
      temp  = sint/temp1;
      temp2 = p.uz*cosp;
      Photon.ux = (p.ux*temp2 - p.uy*sinp)*temp + p.ux*cost;
      Photon.uy = (p.uy*temp2 + p.ux*sinp)*temp + p.uy*cost;
      Photon.uz = -sint*cosp*temp1 + p.uz*cost;
    }else{
      Photon.ux = sint * cosp;
      Photon.uy = sint * sinp;
      Photon.uz = ((p.uz>0) ?  cost : -cost);
    }
    return Photon;
}

typedef struct {
    __m128 d,a,b;
} Packet_v;


inline Packet_v matspin(Packet_v pkt,__m128 trig)
{
    Packet_v res;
    // colums of matrix M
    __m128 M0,M1,M2;
    const __m128 d0=pkt.d, a0=pkt.a, b0=pkt.b;

//	__m128 trig = _mm_shuffle_ps(cost_sint,cosphi_sinphi,_MM_SHUFFLE(1,0,0,1));

	__m128 zero = _mm_setzero_ps();

	__m128 strig = _mm_addsub_ps(zero,trig);	// (-sin phi) (cos phi) (-sin theta) (cos theta)

	__m128 prods = _mm_mul_ps(strig,_mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));
		// prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

	__m128 cp_0_sp_0 = _mm_unpackhi_ps(trig,zero);	// (cos phi) 0 (sin phi) 0

	M0 = _mm_movelh_ps(trig,zero);
	M1 = _mm_shuffle_ps(prods,cp_0_sp_0,_MM_SHUFFLE(3,2,2,1));
	M2 = _mm_shuffle_ps(prods,cp_0_sp_0,_MM_SHUFFLE(3,0,0,3));

    res.d = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(0,0,0,0)));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(0,0,0,0))));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(0,0,0,0))));

    res.a = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(1,1,1,1)));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(1,1,1,1))));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(1,1,1,1))));

    res.b = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(2,2,2,2)));
    res.b = _mm_add_ps(res.b,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2))));
    res.b = _mm_add_ps(res.b,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(2,2,2,2))));

    return res;
}

int main(int argc,char **argv)
{
    const unsigned Npkt  = 10000;
    const unsigned Nspin = 1000;
    const unsigned N = Npkt*Nspin;

    double x,y,z,c;

    unsigned k=0;

    Angle<double> *angle_d  = new Angle<double>[N];
    Angle<float>  *angle_f  = new Angle<float>[N];
    Angle<__m128> *angle_v =  new Angle<__m128>[N];

    double theta,phi;

    for(unsigned i=0;i<N;++i)
    {
        theta = 2.0*boost::math::constants::pi<double>()*uni01(rng);
        phi   = 2.0*boost::math::constants::pi<double>()*uni01(rng);

        angle_f[i].sint = angle_d[i].sint = sin(theta);
        angle_f[i].cost = angle_d[i].cost = cos(theta);
        angle_f[i].sinp = angle_d[i].sinp = sin(phi);
        angle_f[i].cosp = angle_d[i].cosp = cos(phi);

        angle_v[i].trig = _mm_set_ps(angle_f[i].cosp,angle_f[i].sinp,angle_f[i].cost,angle_f[i].sint);
    }

    Packet<float>  *p3f_in = new Packet<float>[Npkt];
    Packet_v       *p3v_in = new Packet_v[Npkt];
    Packet<double> *p3d_in = new Packet<double>[Npkt];

    Packet<float>  *p3f_out = new Packet<float> [Npkt];
    Packet<double> *p3d_out = new Packet<double>[Npkt];
    Packet_v       *p3v_out = new Packet_v[Npkt];

    for(unsigned i=0;i<Npkt;++i)
    {
        x = uni01(rng)-0.5;
        y = uni01(rng)-0.5;
        z = uni01(rng)-0.5;
        c = 1/sqrt(x*x+y*y+z*z);
        p3f_in[i].ux = p3d_in[i].ux = x*c;
        p3f_in[i].uy = p3d_in[i].uy = y*c;
        p3f_in[i].uz = p3d_in[i].uz = z*c;

        p3v_in[i].d = _mm_set_ps(0,z*c,y*c,x*c);
        p3v_in[i].a = normalize(cross(p3v_in[i],_mm_set_ps(0,1,0,0)));
        p3v_in[i].b = cross(p3v_in[i].d,p3v_in[i].a);
//        p3v_in[i].a = getNormalTo(p3v_in[i].d);
//        p3v_in[i].b = cross(p3v_in[i].d,p3v_in[i].a);
    }


    cacheWipe();
    cout << "Type double: " << flush;
    {
        boost::timer::auto_cpu_timer t;
        // run for double type
        Packet<double> p3d;
    
        for(unsigned i=k=0;i<Npkt;++i)
        {
            p3d = p3d_in[i];
            for(unsigned j=0;j<Nspin;++j,++k)
                p3d = timos_spin<double>(angle_d[k].sint,angle_d[k].cost,angle_d[k].sinp,angle_d[k].cosp,p3d);
            p3d_out[i] = p3d;
        }
    }


    cacheWipe();
    cout << "Type float: " << flush;
    {
        boost::timer::auto_cpu_timer t;
        // run for double type
        Packet<float> p3f;
    
        for(unsigned i=k=0;i<Npkt;++i)
        {
            p3f = p3f_in[i];
            for(unsigned j=0;j<Nspin;++j,++k)
                p3f = timos_spin<float>(angle_f[k].sint,angle_f[k].cost,angle_f[k].sinp,angle_f[k].cosp,p3f);
            p3f_out[i] = p3f;
        }
    }

    cacheWipe();
    cout << "Vector implementation: " << flush;
    {
        boost::timer::auto_cpu_timer t;
        Packet_v p3v;
        for(unsigned i=k=0;i<Npkt;++i)
        {
            p3v = p3v_in[i];
            for(unsigned j=0;j<Nspin;++j,++k)
                p3v = matspin(p3v,angle_v[k].trig);
            p3v_out[i] = p3v;
        }
    }

    for(unsigned i=0;i<Npkt;++i)
    {
        if(fabs(getFloat(norm(p3v_out[i].d))-1.0) > 1e-4)
            cerr << "Expecting unit vector, found norm=" << norm(p3v_out[i].d) << endl;
        if(fabs(getFloat(norm(p3v_out[i].a))-1.0) > 1e-4)
            cerr << "Expecting unit vector, found norm=" << norm(p3v_out[i].a) << endl;
        if(fabs(getFloat(norm(p3v_out[i].b))-1.0) > 1e-4)
            cerr << "Expecting unit vector, found norm=" << norm(p3v_out[i].b) << endl;
    }
}
