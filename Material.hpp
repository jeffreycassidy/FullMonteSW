#ifndef MATERIAL_INCLUDED
#define MATERIAL_INCLUDED
#include <immintrin.h>
#include <iostream>
#include <cassert>
#include "Packet.hpp"

#define SUPPRESS_AVX

/** Describes material properties and provides facilities for calculating scattering and reflection/refraction at interfaces.
 * TODO: Incorporate reflection/refraction?
 * TODO: Fix nasty sign convention in propagation vector
 * TODO: Clean up redundant matrix-spin code
 *
 */

class Material {
public:
    typedef class {
    public:
    	float g;				///< g=E[cos(theta)]
    	float one_minus_gg;		///< 1-g^2
    	float one_plus_gg;		///< 1+g^2
    	float recip_2g;			///< 1/2g
    } HGParams;

private:

    static constexpr float const_c0=299.792458;    		///< Speed of light in vacuum (mm/ns) (3e8 m/s = 3e11 mm/s = 3e2 mm/ns)
    float mu_s, mu_a, mu_p, mu_t, n, albedo, absfrac;
    HGParams hgparams;									///< Henyey-Greenstein parameter g and pre-calculated constants

    bool matchedboundary,isscattering;          // 2x1B = 2B
public:


    /// Returns a copy of the Henyey-Greenstein parameters (g and related constants)
    const HGParams& getHGParams() const { return hgparams; }

    /// Returns a __m128 vector holding the Henyey-Greenstein parameters [g, 1/2g, 1+g^2, 1-g^2]
    __m128 getHGParamsSSE() const
    	{ return _mm_set_ps(hgparams.one_minus_gg,hgparams.one_plus_gg,hgparams.recip_2g,hgparams.g); }

    __m128 s_prop;		///< Propagation constant (to be multiplied by physical step length) [ -1, -mu_t, n/c0, 0 ]

	/** Initial propagation vector value.
	 * Has elements [ physical step remaining, dimensionless step remaining, time elapsed, X ]
	 * Values are [ -1/mu_t, -1, 0, 0 ] such that when multiplied by dimensionless step length gives an s_prop vector
	 *
	 * NOTE: Embedded negative signs in the first two elements are because log of a U[0,1) random variable is negative
	 */
    __m128 s_init;


    Material(double mu_a_=0,double mu_s_=0,double g_=0,double n_=1.0,double mu_p_=0.0,bool matchedboundary_=false) :
        mu_s(mu_s_),mu_a(mu_a_),mu_p(mu_p_),mu_t(mu_s_+mu_a_+mu_p_),
        n(n_),
        albedo((mu_s_+mu_p_)/(mu_a_+mu_s_+mu_p_)),
        absfrac(mu_a_/(mu_a_+mu_s_+mu_p_)),
        matchedboundary(matchedboundary_),
        s_prop(_mm_set_ps(0,n_/const_c0,-mu_t,-1)),
        s_init(_mm_set_ps(0,0,1,1/mu_t))
        {
    		setG(g_);
        }

    /// Default operator=
    Material& operator=(const Material& m) =default;

    /// Returns true if the material scatters light
    bool isScattering() const { return isscattering; }

    float getParam_g() const { return hgparams.g; }		///< Returns the g parameter
    float getg()       const { return hgparams.g; }		///< Returns the g parameter TODO: Remove this; redundant
    float getn()       const { return n; }				///< Returns the refractive index
    float getMuA()     const { return mu_a; }			///< Returns the absorption coefficient
    float getMuS()     const { return mu_s; }			///< Returns the scattering coefficient
    float getMuT()     const { return mu_t; }			///< Returns the transport mean free path

    float getAlbedo()              const { return albedo; }		///< Returns the albedo mu_s/(mu_s+mu_a)
    float getAbsorbedFraction()    const { return absfrac; }	///< Returns the absorbed fraction 1-albedo

    bool isMatched() const { return matchedboundary; }			///< Returns true if it's a matched boundary
    void setMatched(bool m=true){ matchedboundary=m; }			///< Sets whether this is treated as a matched boundary


    /// Sets the g parameter and associated pre-calculated constants
    void setG(float g_){
    	hgparams.g=g_;
    	hgparams.one_plus_gg = 1.0+g_*g_;
    	hgparams.one_minus_gg = 1.0-g_*g_;
    	hgparams.recip_2g = 1.0/2.0/g_;
        isscattering = !(g_ == 1.0 || mu_s == 0);
    }

    /** Evaluates the Henyey-Greenstein function 8x in parallel using AVX instructions (wrapper).
     * @param i_rand		Pointer to 8 random U [-1,+1) floats (8 floats, 32B aligned)
     * @param i_uv			Pointer to 8 random 2D unit vectors (2x8 floats, 32B aligned)
     * @param[out] o		Output location, 32 random floats (4x8 floats < cos(theta), sin(theta), cos(phi), sin(phi) > ) (32B aligned)
     */
#ifndef SUPPRESS_AVX
    inline void VectorHG(const float* i_rand,const float* i_uv,float* o) const {
    	VectorHG(_mm256_load_ps(i_rand),_mm256_load_ps(i_uv),_mm256_load_ps(i_uv+8),o);
    }

    inline void VectorHG(__m256 i_rand,std::pair<__m256,__m256> uv,float* o) const {
    	VectorHG(i_rand,uv.first,uv.second,o);
    }

    /// Evaluates the Henyey-Greenstein function 8x in parallel using AVX instructions.
    inline void VectorHG(__m256 i_rand,__m256 uva,__m256 uvb,float* o) const;

    /// Evaluates Henyey-Greenstein ICDF for a scalar float U [-1,1) random number, returning cos(theta)
    inline float ScalarHG(float rnd) const {
    	float costheta;
    	if (hgparams.g != 0)
    	{
    		float t=hgparams.one_minus_gg/(1+hgparams.g*rnd);
    		costheta=hgparams.recip_2g * (hgparams.one_plus_gg-t*t);
    	}
    	else
    		costheta=rnd;
    	assert(costheta <= 1.0 && costheta >= -1.0);
    	return costheta;
    }

    // this used to be in Scatter
    //return matspin(pkt,costheta,cosphi_sinphi);

    /// Evaluates Henyey-Greenstein ICDF for a scalar float U[0,1) random number, returning cos(theta)
    inline float ScalarHG_U01(float rnd) const { return ScalarHG(2.0*rnd-1.0); }

    /// Old scalar scattering calculation.
    //Packet Scatter(Packet pkt,float rnd0,__m128 cosphi_sinphi) const;
#endif
    /// Helper function to print to an ostream
    friend std::ostream& operator<<(std::ostream& os,const Material& mat);
};

/* TODO: Dispose of this old code.
inline Packet matspin(Packet pkt,float costheta,__m128 cosphi_sinphi)
{
    Packet res=pkt;
    // colums of matrix M (appearance in cout output below is transposed)
    __m128 M0,M1,M2;
    const __m128 d0=pkt.d, a0=pkt.a, b0=pkt.b;

    // rows of matrix M
//    M0 = _mm_setr_ps(costheta,sintheta,0,0);
//    M1 = _mm_setr_ps(-sintheta*cosphi,costheta*cosphi,sinphi,0);
//    M2 = _mm_setr_ps(sinphi*sintheta,-sinphi*costheta,cosphi,0);

	__m128 costheta_000 = _mm_set_ss(costheta);

	// calculation from inputs
	__m128 cost_sint = _mm_sqrt_ss(
		_mm_sub_ps(
			_mm_unpacklo_ps(_mm_set_ss(1.0),costheta_000),
			_mm_mul_ss(costheta_000,costheta_000)));

	__m128 trig = _mm_shuffle_ps(cost_sint,cosphi_sinphi,_MM_SHUFFLE(1,0,0,1));

    // rows of matrix M
	// "golden" copy
//    M0 = _mm_setr_ps(costheta,sintheta,0,0);
//    M1 = _mm_setr_ps(-sintheta*cosphi,costheta*cosphi,sinphi,0);	// 0 sinphi (costheta * cosphi)  (-sintheta * cosphi)
//    M2 = _mm_setr_ps(sinphi*sintheta,-sinphi*costheta,cosphi,0);	// 0 cosphi (-sinphi * costheta) (sinphi * sintheta)


	__m128 prods = _mm_mul_ps(strig,_mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));
		// prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

	__m128 cp_0_sp_0 = _mm_unpackhi_ps(trig,zero);	// (cos phi) 0 (sin phi) 0

	M0 = _mm_movelh_ps(trig,zero);
	M1 = _mm_shuffle_ps(prods,cp_0_sp_0,_MM_SHUFFLE(3,2,2,1));
	M2 = _mm_shuffle_ps(prods,cp_0_sp_0,_MM_SHUFFLE(3,0,0,3));

    res.d = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(0,0,0,0)));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(0,0,0,0))));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(0,0,0,0))));

    res.a = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(1,1,1,1)));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(1,1,1,1))));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(1,1,1,1))));

    res.b = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(2,2,2,2)));
    res.b = _mm_add_ps(res.b,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2))));
    res.b = _mm_add_ps(res.b,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(2,2,2,2))));

    return res;
}*/


/** Computes the Henyey-Greenstein function using AVX vectors for a single g value
 *
 * HG ICDF: cos(theta) = F^-1(x) = 	2*x-1											if g == 0
 * 									1/2g*(1 + g^2 - [(1-g^2)/(1 + g * [2x-1] )]^2 )		else
 *
 *
 * @param[in] i_rand	Input random numbers (uses 8) - must be U[-1,+1)
 * @param[in] uva,uvb	Input unit vectors (8 pairs = 16 floats)
 * @param[out] o_uv		Output values (provides 32 floats: cos/sin x 8)
 */

#ifndef SUPPRESS_AVX

inline void Material::VectorHG(__m256 i_rand,__m256 uva,__m256 uvb,float* o) const
{
	__m256 vcos,vsin;

	vcos=i_rand;
	__m256 ones = _mm256_set1_ps(1.0);

	if (hgparams.g != 0.0){
		__m256 v_g = _mm256_set1_ps(hgparams.g);
		__m256 v_recip_2g    = _mm256_set1_ps(hgparams.recip_2g);
		__m256 v_one_plus_gg = _mm256_set1_ps(hgparams.one_plus_gg);
		__m256 v_one_minus_gg= _mm256_set1_ps(hgparams.one_minus_gg);

		// use approximate reciprocal (error ~1e-11 from Intel)
		__m256 t = _mm256_mul_ps(v_one_minus_gg,_mm256_rcp_ps(_mm256_add_ps(ones,_mm256_mul_ps(v_g,vcos))));

		// alternative version of above using exact math
		//__m256 t = _mm256_div_ps(v_one_minus_gg,_mm256_add_ps(ones,_mm256_mul_ps(v_g,vcos)));

		vcos = _mm256_mul_ps(v_recip_2g,_mm256_sub_ps(v_one_plus_gg,_mm256_mul_ps(t,t)));

		// clip to [-1,1]
		vcos = _mm256_max_ps(_mm256_min_ps(vcos,ones),_mm256_sub_ps(_mm256_setzero_ps(),ones));
	}

	// compute sin x = sqrt(1-cos2 x)
	vsin = _mm256_sqrt_ps(_mm256_sub_ps(ones,_mm256_mul_ps(vcos,vcos)));

	// interleave sine/cosine values
	__m256 vl = _mm256_unpacklo_ps(vcos,vsin);
	__m256 vh = _mm256_unpackhi_ps(vcos,vsin);

	// Interleave to get cos(theta) sin(theta) cos(phi) sin(phi)

	// Note: this permutes the values in uva/uvb; if i_uv[0..7] are ABCDEFGH then will give ABEFCDGH
	// watch out for this when debugging - may cause confusion but is valid since they're random numbers (order doesn't matter)

	_mm256_store_ps(o,   _mm256_shuffle_ps(vl,uva,_MM_SHUFFLE(1,0,1,0)));
	_mm256_store_ps(o+8, _mm256_shuffle_ps(vl,uva,_MM_SHUFFLE(3,2,3,2)));
	_mm256_store_ps(o+16,_mm256_shuffle_ps(vh,uvb,_MM_SHUFFLE(1,0,1,0)));
	_mm256_store_ps(o+24,_mm256_shuffle_ps(vh,uvb,_MM_SHUFFLE(3,2,3,2)));
}

#endif

#endif
