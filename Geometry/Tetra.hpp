#pragma once

#include "TetraMeshBase.hpp"
#include "newgeom.hpp"
#include "../Kernels/Software/sse.hpp"

using namespace std;

template<typename FT,std::size_t D>__m128 to_m128(std::array<FT,D> a)
{
	float f[4];
	for(unsigned i=0;i<D;++i)
		f[i]=a[i];
	return _mm_load_ps(f);
}

template<typename FT,std::size_t D>std::array<FT,D> to_array(__m128 v)
{
	float f[4];
	_mm_store_ps(f,v);

	std::array<FT,D> o;
	for(unsigned i=0;i<4;++i)
		o[i] = f[i];
	return o;
}

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

    /// Computes the dot product of the provided vector with each of the four face normals
    inline __m128 dots(__m128) const;

    /// Computes the height of the provided point over each of the four faces (h > 0 -> inside tetra)
    inline __m128 heights(__m128) const;

    /// Checks if a point is within the tetra, with optional tolerance
    inline bool pointWithin(__m128,float eps=0.0f) const;

    StepResult getIntersection(__m128,__m128,__m128 s) const;

    std::array<float,3> face_normal(unsigned i) const;
    float face_constant(unsigned i) const;

} __attribute__ ((aligned(64)));

inline float Tetra::face_constant(unsigned i) const
{
	float f[4];
	_mm_store_ps(f,C);
	return f[i];
}

inline std::array<float,3> Tetra::face_normal(unsigned i) const
{
	float x[4],y[4],z[4];
	_mm_store_ps(x,nx);
	_mm_store_ps(y,ny);
	_mm_store_ps(z,nz);
	return std::array<float,3>{ x[i], y[i], z[i] };
}

inline __m128 Tetra::dots(const __m128 d) const
{
	return _mm_add_ps(
			_mm_add_ps(
					_mm_mul_ps(ny,_mm_shuffle_ps(d,d,_MM_SHUFFLE(1,1,1,1))),
					_mm_mul_ps(nz,_mm_shuffle_ps(d,d,_MM_SHUFFLE(2,2,2,2)))
			),
			_mm_mul_ps(nx,_mm_shuffle_ps(d,d,_MM_SHUFFLE(0,0,0,0))));
}

inline __m128 Tetra::heights(const __m128 p) const
{
	return _mm_sub_ps(dots(p),C);
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


/** Verifies that a point is within the specified tetrahedron, using tolerance eps.
 */
inline bool Tetra::pointWithin(__m128 p,float eps) const
{
	__m128 h = heights(p);
    __m128 cmpWithTolerance = _mm_add_ps(h,_mm_set1_ps(eps));

    return _mm_movemask_ps(cmpWithTolerance) == 0;			// movemask shows if top (sign) bit is set. 0 means all positive.
}

//
//if (_mm_movemask_ps(cmpWithTolerance))
//{
//	cout << "NOTE: Point is not within tetra" << endl;
//	float f[4];
//	_mm_store_ps(f,dot);
//	cout << "  Dot: ";
//	for(unsigned i=0;i<4;++i)
//		cout << f[i] << ' ';
//	_mm_store_ps(f,cmpWithTolerance);
//	cout << endl << "  With tolerance: ";
//	for(unsigned i=0;i<4;++i)
//		cout << "  " << f[i] << ' ';
//	cout << endl;
//}

