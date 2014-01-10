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
    const unsigned N=10000000;
    const uint32_t *p;
    RNG_SFMT rng(1024);
    __m128 t=_mm_setzero_ps();

    __m128 *rand_op = new __m128[N];

    cout << "Using SFMT" << endl;
    {
        boost::timer::auto_cpu_timer timer;

        for(unsigned i=0;i<N;++i)
            t = _mm_add_ps(t,rng.draw_m128f2_uvect());
    }
    cout << t << endl;

/*    ofstream os("rands2.out.txt");
    for(unsigned i=0;i<N;++i)
    {
        _mm_store_ps(f,rand_op[i]);
        os << f[0] << ' ' << f[1] << endl;
    }
    os.close();*/

    t = _mm_setzero_ps();

    cout << "Using Boost Mersenne Twister & sin/cos" << endl;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
        {
            f[0] = sinf(2*3.141592654*uni01(boost_rng));
            f[1] = cosf(2*3.141592654*uni01(boost_rng));
            t = _mm_add_ps(t,_mm_load_ps(f));
        }
    }
    cout << t << endl;

    cout << "======================================== Generating floats " << endl;
    cout << "Boost" << endl;
    float sum=0;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
            sum += uni01(boost_rng);
    }
    cout << sum << endl;

    __m128 vsum=_mm_setzero_ps();
    cout << "SFMT using __m128" << endl;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
            vsum = _mm_add_ps(vsum,rng.draw_m128f1_u01());
    }
    cout << vsum << endl;

    sum=0;
    cout << "SFMT using float" << endl;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
            sum += rng.draw_float_u01();
    }
    cout << sum << endl;


    cout << "======================================== Generating doubles " << endl;
    cout << "Boost" << endl;
    double sum_d=0;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
            sum_d += uni01d(boost_rng);
    }
    cout << sum_d << endl;

    sum_d=0;
    cout << "SFMT using double type" << endl;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
            sum_d += rng.draw_double_u01();
    }
    cout << sum_d << endl;

    __m128d vsum_d=_mm_setzero_pd();

    cout << "SFMT using __m128d type" << endl;
    {
        boost::timer::auto_cpu_timer timer;
        for(unsigned i=0;i<N;++i)
            vsum_d = _mm_add_sd(vsum_d,rng.draw_m128d1_u01());
    }
    double d_tmp;
    _mm_store_sd(&d_tmp,vsum_d);
    cout << d_tmp << endl;

//    ofstream os("rands2.out.txt");
//    for(unsigned i=0;i<N;++i)
//    {
//        _mm_store_ps(f,rand_op[i]);
//        os << f[0] << ' ' << f[1] << ' ' << f[2] << endl;
//    }
//    os.close();
}
