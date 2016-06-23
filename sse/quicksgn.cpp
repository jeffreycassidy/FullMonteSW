#include <iostream>
#include <boost/timer/timer.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

using namespace std;

boost::random::mt19937 rng;
boost::random::uniform_01<double> uni01;

bool verify(const double* i,const double *o,unsigned N)
{
    bool check=true;
    for(unsigned j=0;j<N;++j)
    {
        if (i[j<<1] * i[(j<<1)+1] * o[j] < 0)
        {
            cerr << "Error: input (" << i[j<<1] << "," << i[(j<<1)+1] << ") => " << o[j] << endl;
            check=false;
        }
    }
    return check;
}

void cacheBlast(unsigned N=100000000)
{
    char *p = new char[N];
    for(unsigned i=0;i<N;++i)
        p[i]=i;
    delete[] p;
}

int main(int argc,char **argv)
{
    const unsigned long long msb64 = (1ULL << 63ULL);

    double *in_val = new double[20000000];

    for(double* i=in_val; i<in_val+20000000; ++i)
        *i = 2*uni01(rng)-1.9;

    double *o_bit = new double[10000000];
    double *o_cond = new double[10000000];

    cout << "Performing sign-copy 10,000,000 times:" << endl;

    boost::timer::cpu_timer t;

    cout << "  using bit-hacking: " << flush;
    cacheBlast();
    t.start();

    for(uint64_t *i=(uint64_t*)in_val,*o=(uint64_t*)o_bit; i<(uint64_t*)in_val+10000000; i+=2,++o)
        *o = *(i+1) ^ (*i & msb64);

    t.stop();
    cout << t.elapsed().wall/1000 << " us " << (verify(in_val,o_bit,10000000) ? "OK" : "Error") << endl;


    cout << "  using conditionals: " << flush;
    cacheBlast();

    t.start();

    // performance drops as the branch probability nears p=0.5; quite fast for p near 1 or 0
    for(double *i=in_val,*o=o_cond; i<in_val+10000000; i+=2,++o)
        *o = *i>0 ? *(i+1) : -*(i+1);
    t.stop();

    cout << t.elapsed().wall/1000 << " us " << (verify(in_val,o_cond,10000000) ? "OK" : "Error") << endl;
}
