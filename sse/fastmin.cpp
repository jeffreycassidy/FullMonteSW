#include "timebench.hpp"
#include <iomanip>
#include <cmath>

#include <boost/timer/timer.hpp>

#include <stdlib.h>

using namespace std;

// _mm_set_ps(1,2,3,4) prints as <4,3,2,1>

// _mm_store_ps(f,m)
// f[0]=m0, f[1]=m1, f[2]=m2, f[3]=m3


// _mm_set_ps(a,b,c,d) does r[3]=a, r[2]=b, r[1]=c, r0=d

// f[0]=a f[1]=b f[2]=c f[3]=d

// stored in memory <r3,r2,r1,r0> = <a,b,c,d>

// index bit 0 set if B,D are min (B < ACD || D < ABC)
// index bit 1 set if C,D are min

inline float refmin(float* x)
{
    float x_min=1e9;
    for(float* p=x; p<x+4; ++p)
        if(*p<x_min) x_min=*p;
    return x_min;
}

/*inline float fastmin3(float *f,float *x)
{
    unsigned idx;

    __m128 abcd = _mm_load_ps(f);
    unsigned i _mm_movemask_ps(_mm_cmple(abcd,_mm_shuffle_ps(abcd,abc,_MM_SHUFFLE())));

    *f = x[idx];
}*/

int main(int argc,char **argv)
{
    const unsigned N=100000000;
    unsigned errct;

    float* const refin = new float[6*N];
    float* const refout=refin+4*N;
    float* const testout=refout+N;
//    __m128* const testin=new __m128[N];

    float *q=refin;

    // create random stimulus
    for(unsigned i=0;i<N;++i)
    {
        q[0] = rand()/65536;
        q[1] = rand()/65536;
        q[2] = rand()/65536;
        q[3] = rand()/65536;
//        testin[i] = _mm_set_ps(q[0],q[1],q[2],q[3]);
        q += 4;
    }

    cacheWipe();

    // run golden sim
    cout << "Reference version: ";
    {
        boost::timer::cpu_timer t;
        t.start();
        for(float *y=refout,*x=refin; y<refout+N;x += 4,++y)
            *y=refmin(x);
        t.stop();
        cout << boost::timer::format(t.elapsed()) << endl;
    }

    cacheWipe();

    // run test sim
    cout << endl << "SSE version: ";
    {
        boost::timer::cpu_timer t;
        float *y,*x;
        t.start();
        for(x=refin,y=testout; x<refin+4*N; x+=4,++y)
            fastmin(y,_mm_load_ps(x));
        t.stop();
        cout << boost::timer::format(t.elapsed()) << endl;
    }
    errct=0;
    for(unsigned i=0;i<N;++i)
        errct += fabs(testout[i]-refout[i])>1e-8;


    cacheWipe();

    // run test sim
    cout << endl << "SSE version 2: ";
    {
        boost::timer::cpu_timer t;
        float *y,*x;
        t.start();
        for(x=refin,y=testout; x<refin+4*N; x+=4,++y)
            fastmin2(y,x);
        t.stop();
        cout << boost::timer::format(t.elapsed()) << endl;
    }

    errct=0;
    for(unsigned i=0;i<N;++i)
        errct += fabs(testout[i]-refout[i])>1e-8;

    cout << "errors: " << errct << endl;
    delete[] refin;
}
