#include "timebench.hpp"
#include <emmintrin.h>
#include <xmmintrin.h>
#include <mmintrin.h>
#include <pmmintrin.h>

#include "../sse.hpp"
//#include "SFMT.h"
#include "../random.hpp"

#include <fstream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/timer/timer.hpp>

boost::random::mt19937              boost_rng;
boost::random::uniform_01<float>    uni01;
boost::random::uniform_01<double>   uni01d;

int main(int argc,char **argv)
{
    float f[4] __attribute__((aligned(16))) = { 0,0,0,0 } ;
    double d[2] __attribute__((aligned(16)));
    const unsigned N=1000000;
    const uint32_t *p;
    RNG_SFMT rng(1024);
    __m128 t=_mm_setzero_ps();

    ofstream os;

    os << setprecision(7);

    __m128 *rand_op = new __m128[N];

    cout << "======================================== floats in scalar mode" << endl;

    os.open("float.scalar.txt");
    for(unsigned i=0;i<N;++i)
        os << rng.draw_float_u01() << endl;
    os.close();

	cout << "======================================== doubles in scalar double mode" << endl;

    os.open("double.scalar.txt");
    for(unsigned i=0;i<N;++i)
        os << rng.draw_double_u01() << endl;
    os.close();

    cout << "======================================== 2D unit vectors in __m128 float mode" << endl;

    os.open("uvect2f.m128f.txt");
    for(unsigned i=0;i<N;++i)
    {
        _mm_store_ps(f,rng.draw_m128f2_uvect());
        os << f[0] << ' ' << f[1] << endl;
    }
    os.close();

    cout << "======================================== 3D unit vectors in __m128 float mode" << endl;

    os.open("uvect3f.m128f.txt");
    for(unsigned i=0;i<N;++i)
    {
        _mm_store_ps(f,rng.draw_m128f3_uvect());
        os << f[0] << ' ' << f[1] << ' ' << f[2] << endl;
    }
    os.close();
    
    cout << "======================================== __m128d2 doubles" << endl;

    os.open("double2.m128d.txt");
    for(unsigned i=0;i<N;++i)
    {
        _mm_store_pd(d,rng.draw_m128d2_u01());
        os << d[0] << ' ' << d[1] << endl;
    }
    os.close();


    cout << "======================================== __m128f1 scalar floats" << endl;
    os.open("float.m128f.txt");
    for(unsigned i=0;i<N;++i)
    {
        _mm_store_ss(f,rng.draw_m128f1_u01());
        os << *f << endl;
    }
    os.close();
}
