#include <xmmintrin.h>
#include <pmmintrin.h>
#include <emmintrin.h>

#include <stdio.h>

float __attribute__ (( aligned(16) )) M[16] = {
    1.0, 2.0, 3.0, 4.0,
    5.0, 6.0, 7.0, 8.0,
    9.0, 10.0,11.0,12.0,
    13.0,14.0,15.0,16.0 };
float *P[4] = { M,M+4,M+8,M+12 };

float Mt[16];
float *Pt[4] = { Mt,Mt+4,Mt+8,Mt+12 };

void printvec(float *V);
void printmat(float **P);

int main(int argc,char **argv)
{
    __m128 a0,a1,a2,a3,t0,t1,t2,t3,at0,at1,at2,at3;

    printf("Original matrix:\n");
    printmat(P);

    a0 = _mm_load_ps(P[0]);
    a1 = _mm_load_ps(P[1]);
    a2 = _mm_load_ps(P[2]);
    a3 = _mm_load_ps(P[3]);

    // displays lowest first ( x0 | x1 | x2 | x3 )

    // _MM_SHUFFLE order goes most sig -> least sig position
    // which is opposite display order f[0] f[1] f[2] f[3]

    t0 = _mm_shuffle_ps(a0,a1,_MM_SHUFFLE(1,0,1,0));
    t0 = _mm_shuffle_ps(t0,t0,_MM_SHUFFLE(3,1,2,0));

    t1 = _mm_shuffle_ps(a0,a1,_MM_SHUFFLE(3,2,3,2));
    t1 = _mm_shuffle_ps(t1,t1,_MM_SHUFFLE(3,1,2,0));

    t2 = _mm_shuffle_ps(a2,a3,_MM_SHUFFLE(1,0,1,0));
    t2 = _mm_shuffle_ps(t2,t2,_MM_SHUFFLE(3,1,2,0));

    t3 = _mm_shuffle_ps(a2,a3,_MM_SHUFFLE(3,2,3,2));
    t3 = _mm_shuffle_ps(t3,t3,_MM_SHUFFLE(3,1,2,0));

    at0 = _mm_movelh_ps(t0,t2);
    at2 = _mm_movelh_ps(t1,t3);
    at1 = _mm_movehl_ps(t2,t0);
    at3 = _mm_movehl_ps(t3,t1);

    printf ("Transposed (?) matrix: \n");

    printf ("at0 "); printvec(&at0);
    printf ("at1 "); printvec(&at1);
    printf ("at2 "); printvec(&at2);
    printf ("at3 "); printvec(&at3);

    return 0;
}

void printvec(float *V)
{
    unsigned i;
    for(i=0;i<4;++i)
        printf("%10.2f ",V[i]);
    printf("\n");
}

void printmat(float **P)
{
    unsigned i;
    for(i=0;i<4;++i)
    {
        printvec(P[i]);
        printf("\n");
    }
}
