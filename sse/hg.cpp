#include <emmintrin.h>
#include <pmmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

#include <immintrin.h>

#include <iostream>

using namespace std;

const unsigned N=10<<20;

// ICDF for HG: cos(theta) = 1/2/g*[1+g*g-((1-g*g)/(1-gq))^2]

// stored constants: g, k0 = 1/2/g*(1+g*g), k1 = sqrt(1/2/g)*(1-g*g)

// need to eval k0 - [k1/(1+gp)]^2

ostream& operator<<(ostream& os,__m256 x)
{
    union {
        __m256 v;
        float f[8];
    } t;
    t.v = x;
    for(unsigned i=0;i<8;++i)
        os << t.f[i] << (i==7 ? '\0':' ');
    return os;
}

/*

inline __m256 HG_AVX(__m256 r,__m128 consts)
{
    __m256 k0 = _mm256_shuffle_ps(consts,);
    __m256 g  = _mm256_shuffle_ps(consts,0);

    if ()
        return r;

    __m256 k1 = _mm256_shuffle_ps(consts,);

    __m256 s = _mm256_mul_ps(

    __m256 a = _mm256_div_ps(k1,

    _mm256_sub_ps(,_mm256_mul_ps(a,a));
}*/

int main(int argc,char **argv)
{
    union {
        __m256 v;
        float f[8];
        __m128 s;
    } t;

    for(unsigned i=0;i<N;++i)
        t.f[i]=i;


    cout << "Vector is: " << t.v << endl;

//    t.v = _mm256_shuffle_ps(t.v,t.v,0);
//    t.v = _mm256_permute_ps(t.v,0);
//    t.v = _mm256_permute_ps(t.v,0);
//    t.v = _mm256_unpacklo_ps(t.v,t.v);
//    t.v = _mm256_unpacklo_ps(t.v,t.v);
    t.v = _mm256_insertf128_ps(t.v,_mm_shuffle_ps(t.s,t.s,0),1);

    cout << "Post-shuffle: " << t.v << endl;


/*    for(unsigned i=0;i<N;++i)
        float

    for(unsigned i=0;i<N;++i)
        cout <<  << endl;*/
}
