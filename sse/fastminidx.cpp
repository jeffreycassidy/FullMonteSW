#include "../sse.hpp"
#include <iostream>
#include <iomanip>

#include <boost/timer/timer.hpp>

using namespace std;

string printBin(unsigned x,unsigned Nb)
{
    string str(' ',Nb+1);
    for(unsigned i=0;i<Nb;++i,x>>=1)
        str[i] = (x & 1 ? '1' : '0');
    return str;
}

const unsigned N=10000000;

int main(int argc,char **argv)
{
    float *f = new float[N*4];
    float *p=f;
    float min_ref,min_test;
    unsigned min_i;

    float vec_i[4] = { 1,0,0,0 };
    float vec_j[4] = { 0,1,0,0 };
    float vec_k[4] = { 0,0,1,0 };

    cout << "BASES: " << getBasis(0) << " " << getBasis(1) << " " << getBasis(2) << endl;

    for(unsigned i=0;i<100;++i)
    {
        float tmp[4] = { rand(),rand(),rand(),0 };
        __m128 m = _mm_load_ps(tmp);
        __m128 n = normalize(m);
        cout << m << " " << n << " " << dot(n,n) << endl;
    }

    __m128 mi=getBasis(0), mj=getBasis(1), mk=getBasis(2);

    __m128 blah;

    cout << mi << " " << mj << " " << mk << endl;
    cout << cross(mj,mk) << " " << cross(mk,mi) << " " << cross(mi,mj) << endl;

    cout << "Basis(1)=" << getBasis(1) << endl;
    cout << "MinAbsIdx=" << getMinAbsIndex3(getBasis(1)) << endl;
    cout << "Basis for cross=" << (blah=getBasis(getMinAbsIndex3(getBasis(1)))) << endl;
    cout << "Crossprod=" << (blah=cross(blah,getBasis(1))) << endl;
    cout << "Normalize=" << normalize(blah) << endl;

    cout << "Normals to i=" << getNormalTo(_mm_set_ps(0,1,0,1)) << " j=" << getNormalTo(getBasis(1)) << " k=" << getNormalTo(getBasis(2)) << endl;

    __m128 test=_mm_set_ps(3.0,2.0,1.0,0.0);

    cout << "Select from i%4: ";

    float f_sum;

    {
        boost::timer::auto_cpu_timer t;
        for(unsigned i=0;i<N;++i)
            f_sum += selectFrom(test,i%4);
    }

    cout << f_sum << endl;

    cout << "Loop and mod overhead: ";

    unsigned tmp;
    {
        boost::timer::auto_cpu_timer t;
        for(unsigned i=0;i<N;++i){ tmp+=i%4; }
    }
    cout << tmp;

    for(unsigned i=0;i<4;++i)
        cout << selectFrom(test,i) << " ";
    cout << endl;

    for(unsigned i=0;i<4*N;++i)
        f[i] = rand();

    for(unsigned i=0;i<N;++i,p+=4)
    {
        min_ref = min(min(p[0],p[1]),min(p[2],p[3]));
        min_i = getMinIndex4(_mm_load_ps(p));

        if (min_i>3)
            cerr << "Invalid index" << endl;
        else if ((min_test=p[min_i]) != min_ref)
            cout << setw(10) << min_ref << "  " << setw(10) << min_test << endl;
    }

    //for(unsigned i=0;i<16;++i)
//        cout << "int " << setw(2) << dec << i << "   hex " << setw(1) << hex << i << " " << printBin(i,4) << "   min index" << getMinIndex(i) << endl;
}
