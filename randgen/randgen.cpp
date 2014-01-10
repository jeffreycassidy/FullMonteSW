#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/taus88.hpp>
#include <cstdlib>
#include <string>
#include <sys/time.h>
#include <iomanip>
#include <iostream>
#include <stdlib.h>

extern "C" {
#define DSFMT_MEXP 19937
#include "dSFMT.h"
}

using namespace std;

class RandGen {
    public:

    template<class T>void drawN(unsigned,T*);

    virtual string getName() const=0;
    virtual void draw(double*)=0;
    virtual void draw(unsigned*)=0;

    bool bernoulli(double);
    double exponential();
};

template<class T>void RandGen::drawN(unsigned N,T* rands)
{
    for(unsigned i=0;i<N;++i)
        draw(rands++);
}

class RandGen_CStd : public RandGen {
    public:

    string getName() const { return string("C Standard Library"); }
    void draw(double* x){ *x=rand()/(double)RAND_MAX; }
    void draw(unsigned* x){ *x=rand(); }
};

//typedef dsfmt_t aligned_dsfmt_t __attribute((aligned(16)));
typedef dsfmt_t aligned_dsfmt_t;

class RandGen_dSFMT : public RandGen {
    aligned_dsfmt_t* p;
    public:
    RandGen_dSFMT(aligned_dsfmt_t* p_) : p(p_) { dsfmt_init_gen_rand(p,1); }
    string getName() const { return string("dSFMT 2.1"); }
    void draw(double* x){ *x=dsfmt_genrand_open_open(p); }
    void draw(unsigned* x){ *x=dsfmt_genrand_uint32(p); }
};


template<class BoostRandGen>class RandGen_Boost : public RandGen {
    string name;
    BoostRandGen rng;

    static boost::random::uniform_01<double> uni01;

    public:
    RandGen_Boost(string name_) : name(name_){};
    string getName() const { return name; }

    void draw(double* d){   *d = uni01(rng); };
    void draw(unsigned* x){ *x = rng(); };
};


int main(int argc,char **argv)
{
    struct timeval start,end;
    aligned_dsfmt_t dsfmt;
    unsigned N=100000000;
    
    RandGen_CStd rng_std;
    RandGen_Boost<boost::random::mt19937> rng_mt19937("Boost MT19937");
    RandGen_Boost<boost::random::mt11213b> rng_mt11213b("Boost MT11213b");
    RandGen_Boost<boost::random::taus88> rng_taus88("Boost Taus88");
    RandGen_dSFMT rng_dsfmt(&dsfmt);

    RandGen *rngs[] = {
        &rng_std,
        &rng_mt19937,
        &rng_mt11213b,
        &rng_taus88,
        &rng_dsfmt,
        NULL };

//    dsfmt_init_gen_rand(&dsfmt,1);
//    cout << dsfmt_genrand_open_open(&dsfmt) << endl;

    cout << "Generating type double" << endl;
    double *x;
    if (posix_memalign((void**)&x,16,N*sizeof(double)) != 0)
    {
        cerr << "allocation failed" << endl;
        return -1;
    }
    for(RandGen **p=rngs; *p != NULL; p++)
    {
        cout << setw(20) << (*p)->getName() << " ";
        gettimeofday(&start,NULL);
        (*p)->drawN(N,x);
//        sort(x,x+N);
        gettimeofday(&end,NULL);
        cout << setw(10) << (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec) << " usec" << endl;
    }

    cout << setw(20) << "Array dSFMT: " << endl;

    gettimeofday(&start,NULL);
    dsfmt_fill_array_close_open(&dsfmt,x,N);
//    sort(x,x+N);
    gettimeofday(&end,NULL);
    cout << setw(10) << (end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec) << " usec" << endl;
}
