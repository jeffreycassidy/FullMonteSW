#include <mmintrin.h>
#include <emmintrin.h>
#include <stdio.h>

typedef union {
    __m128 m;
    float f[4];
} ssereg_t;

// useful: _mm_set_ps(z,x,y,w)

int main(int argc,char **argv)
{
    ssereg_t a;
    float test[4]={1.0,2.0,3.0,4.0};
    float ret[4];

    // SSE packed single: Float 0 appears in low-order address
    a.m = _mm_load_ps(test);
    _mm_store_ps(ret,a.m);

    printf("a.m[0] = ??\na.f[0] = %f\nret[0] = %f\nShould be 1.0\n",a.f[0],ret[0]);

    // Intel little-endian ordering (begins with little end): high-order byte goes last
    long i=0xDEADBEEF;
    char* s=(char*)&i;
    unsigned s0=s[0],s1=s[1],s2=s[2],s3=s[3];
    printf("i=%ld\ni[0]=%x\ni[1]=%x\ni[2]=%x\ni[3]=%x\n",i,s0,s1,s2,s3);
    return 0;
}
