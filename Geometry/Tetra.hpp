#pragma once

#include "TetraMeshBase.hpp"
#include "newgeom.hpp"
#include "../Kernels/Software/sse.hpp"

using namespace std;

// TODO: Move this to Kernels/Software since it's really SSE-specific

typedef struct { 
        __m128 Pe;          // 4x16B = 64 B
        __m128 distance;
        int IDfe;           // 4B
        unsigned IDte;      // 4B
        int idx;            // 4B
        bool hit;           // 1B
    } StepResult;



/** Tetra geometry model, optimized for SSE instructions.
 * The x,y,z, and C (constant) components of the normal for the four faces are stored together to facilitate matrix multiplication.
 *
 * Normals are oriented so that points inside the tetra have a positive altitude h= dot(p,n)-C
 * Face IDs are such that if IDf > 0 then m_faces[IDf] holds the face. If IDf <0 then m_faces[-IDf] holds the inverted face.
 */

struct Tetra {
    __m128 nx,ny,nz,C;      	/// Normal components (4 x 16B = 64B)
    TetraByFaceID   IDfs;   	/// Face IDs (4 x 4B = 16 B)
    unsigned adjTetras[4];  	/// Adjacent tetras for each face (4 x 4B = 16 B)
    unsigned faceFlags=0;		/// Flags for each face (4B = 32b -> 8b each)
    unsigned matID;         	/// Material ID for this tetra (4 B)


    bool pointWithin(std::array<double,3> p,float eps=0.0f) const
    {
    	__m128 pv = _mm_set_ps(0.0,p[2],p[1],p[0]);
    	return pointWithin(pv);
    }

    bool pointWithin(__m128,float eps=0.0f) const;

    __m128 heights(__m128) const;
    std::array<float,4> heights(std::array<float,3>) const;

    __m128 dots(__m128) const;
    std::array<float,4> dots(std::array<float,3>) const;

    StepResult getIntersection(__m128,__m128,__m128 s) const;
} __attribute__ ((aligned(64)));

inline __m128 Tetra::dots(const __m128 d) const
{
	return _mm_add_ps(
			_mm_add_ps(
					_mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))),
					_mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2)))
			),
			_mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0))));
}

inline std::array<float,4> Tetra::dots(const std::array<float,3> d) const
{
	std::array<float,4> a{ d[0], d[1], d[2], 0.0f};
	_mm_store_ps(a.data(), dots(_mm_load_ps(a.data())));
	return a;
}

inline __m128 Tetra::heights(const __m128 p) const
{

    // calculate dot = n (dot) d, height = n (dot) p - C
    __m128 h1 =             _mm_mul_ps(nx,_mm_shuffle_ps(p,p,_MM_SHUFFLE(0,0,0,0)));

    h1 = _mm_add_ps(h1, _mm_mul_ps(ny,_mm_shuffle_ps(p,p,_MM_SHUFFLE(1,1,1,1))));

    h1 = _mm_add_ps(h1, _mm_mul_ps(nz,_mm_shuffle_ps(p,p,_MM_SHUFFLE(2,2,2,2))));

    // height (=C - p dot n) should be negative if inside tetra, may occasionally be (small) positive due to numerical error
    // dot negative means facing outwards
    h1 = _mm_sub_ps(C,h1);

	return h1;
}

inline std::array<float,4> Tetra::heights(std::array<float,3> pa) const
{
	std::array<float,4> a{ pa[0], pa[1], pa[2], 0 };
	_mm_store_ps(a.data(),heights(_mm_load_ps(a.data())));
	return a;
}

inline StepResult Tetra::getIntersection(const __m128 p,const __m128 d,__m128 s) const
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
    dist = _mm_blendv_ps(
    			_mm_set1_ps(std::numeric_limits<float>::infinity()),
				dist,
				dot);

    // at most three of the dot products should be negative

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
    result.distance=_mm_max_ps(_mm_setzero_ps(),result.distance);
    result.Pe = _mm_add_ps(p,_mm_mul_ps(d,result.distance));

    return result;
}
