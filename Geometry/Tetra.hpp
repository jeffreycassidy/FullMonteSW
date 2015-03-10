#pragma once

#include "TetraMeshBase.hpp"
#include "newgeom.hpp"
#include "../sse.hpp"

using namespace std;

typedef struct { 
        __m128 Pe;          // 4x16B = 64 B
        __m128 distance;
        int IDfe;           // 4B
        unsigned IDte;      // 4B
        int idx;            // 4B
        bool hit;           // 1B
    } StepResult;

struct Tetra {
    __m128 nx,ny,nz,C;      // 4 x 16B = 64B
    TetraByFaceID   IDfs;   // 4 x 4B = 16 B
    unsigned adjTetras[4];  // 4 x 4B = 16 B
    unsigned matID;         // 4 B

    bool pointWithin(__m128) const;

    StepResult getIntersection(__m128,__m128,__m128 s) const;
    
} __attribute__ ((aligned(64)));

inline StepResult Tetra::getIntersection(__m128 p,__m128 d,__m128 s) const
{
    StepResult result;

    result.idx=-1;

    s = _mm_shuffle_ps(s,s,_MM_SHUFFLE(0,0,0,0));

    // calculate dot = n (dot) d, height = n (dot) p - C
    __m128 dot    =             _mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0)));
    __m128 h1 =             _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));

    dot    = _mm_add_ps(dot,    _mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))));
    h1 = _mm_add_ps(h1, _mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));

    dot    = _mm_add_ps(dot,    _mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2))));
    h1 = _mm_add_ps(h1, _mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));

    // height (=C - p dot n) should be negative if inside tetra, may occasionally be (small) positive due to numerical error
    // dot negative means facing outwards
    h1 = _mm_sub_ps(C,h1);


    // dist = height/dot
    __m128 dist = _mm_div_ps(h1,dot);

//  selects dist where dist>0 and dot<0 (facing outwards), s otherwise
    // very, very rarely ( < 1e-8? ) gives an error where no intersection is found
    // used to be s below instead of infinity - would return at most s; gave wrong nearest-face results though
    // dist = _mm_blendv_ps(a,b,mask)
    //  dist_i = (mask_i & 0x80000000) ? b_i : a_i;
    dist = _mm_blendv_ps(_mm_set1_ps(std::numeric_limits<float>::infinity()),dist,_mm_and_ps(_mm_cmpgt_ps(dist,_mm_setzero_ps()),dot));

    // at most three of the dot products should be negative
    // ideally none of the heights should be negative (assuming we're in the tetra)

    //      height  dot     h/dot   meaning
    //      +       +       +       OK: inside, facing away (no possible intersection)
    //      +       -       -       OK: inside, facing towards (intersection possible)
    //      -       +       -       OK: outside, facing in (this is the entry face with roundoff error, no possible intersection)
    //      -       -       +       ERROR: outside, facing out (problem!! this must be the entry face, but exiting!)

    // require p dot n - C > 0 (above face) and d dot n < 0

    pair<unsigned,__m128> min_idx_val = getMinIndex4p(dist);

    result.hit = _mm_ucomilt_ss(min_idx_val.second,s);


    result.IDfe = IDfs[min_idx_val.first&3];
    result.IDte = adjTetras[min_idx_val.first&3];
    result.idx = min_idx_val.first;					// will be 4 if no min found
    result.distance=_mm_min_ps(min_idx_val.second,s);
    result.Pe = _mm_add_ps(p,_mm_mul_ps(d,result.distance));

    return result;
}
