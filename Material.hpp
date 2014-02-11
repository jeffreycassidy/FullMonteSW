#ifndef MATERIAL_INCLUDED
#define MATERIAL_INCLUDED
#include <immintrin.h>
#include <iostream>

#include "Packet.hpp"

class Material {
public:
    typedef struct {
    	float g;
    	float one_minus_gg;
    	float one_plus_gg;
    	float recip_2g;
    } HGParams;

private:

    static const float const_c0;    ///< Speed of light in vacuum (mm/ns) (3e8 m/s = 3e11 mm/s = 3e2 mm/ns)
    float mu_s, mu_a, mu_p, mu_t, n, albedo, absfrac; // 11x4 = 40B
    HGParams hgparams;

    bool matchedboundary,isscattering;          // 2x1B = 2B
public:


    /// Returns a copy of the Henyey-Greenstein parameters (g and related constants)
    const HGParams& getHGParams() const { return hgparams; }
    //__m128 getHGParamsSSE() const { return _mm_set_ps(one_minus_gg,one_plus_gg,recip_2g,g); }

    __m128 s_prop;		///< Propagation constant
    __m128 s_init;		///< Initial propagation value

    Material(double mu_a_=0,double mu_s_=0,double g_=0,double n_=1.0,double mu_p_=0.0,bool matchedboundary_=false) :
        mu_s(mu_s_),mu_a(mu_a_),mu_p(mu_p_),mu_t(mu_s_+mu_a_+mu_p_),
        n(n_),
        albedo((mu_s_+mu_p_)/(mu_a_+mu_s_+mu_p_)),
        absfrac(mu_a_/(mu_a_+mu_s_+mu_p_)),
        matchedboundary(matchedboundary_),
        isscattering(!(g_ == 1.0 || mu_s_ == 0)),
        s_prop(_mm_set_ps(0,n_/const_c0,-mu_t,-1)),
        s_init(_mm_set_ps(0,0,-1,-1/mu_t))
        {
    		setG(g_);
        }

    bool isScattering() const { return isscattering; }

    float getParam_g() const { return hgparams.g; }
    float getn()       const { return n; }
    float getMuA()     const { return mu_a; }
    float getMuS()     const { return mu_s; }
    float getMuT()     const { return mu_t; }
    float getg()       const { return hgparams.g; }

    float getAlbedo()              const { return albedo; }
    float getAbsorbedFraction()    const { return absfrac; }

	Packet Scatter(Packet pkt,float rnd0,__m128 cosphi_sinphi) const;

    bool isMatched() const { return matchedboundary; }
    void setMatched(bool m=true){ matchedboundary=m; }

    void setG(float g_){
    	hgparams.g=g_;
    	hgparams.one_plus_gg = 1.0+g_*g_;
    	hgparams.one_minus_gg = 1.0-g_*g_;
    	hgparams.recip_2g = 1.0/2.0/g_;
    }

    Material& operator=(const Material& m);

    inline void VectorHG(const float* i_rand,const float* i_uv,float* o) const {
    	VectorHG(_mm256_load_ps(i_rand),_mm256_load_ps(i_uv),_mm256_load_ps(i_uv+8),o);
    }

    inline void VectorHG(__m256 i_rand,__m256 uva,__m256 uvb,float* o) const;

    friend std::ostream& operator<<(std::ostream& os,const Material& mat);
};

/*inline Packet Material::Scatter(Packet pkt,float rnd0,__m128 cosphi_sinphi) const
{
    float costheta,P=2*rnd0-1;

    float t=one_minus_gg/(1+g*P);

    // choose angles: HG function for component along d0, uniform circle for normal components
    costheta = g==0 ? P : recip_2g * (one_plus_gg-t*t);
    assert (costheta <= 1.0 && costheta >= -1.0);

    return matspin(pkt,costheta,cosphi_sinphi);
}*/


// r0,r1 are the random numbers
/*Packet matspin(Packet pkt,double costheta,double sintheta,double cosphi,double sinphi)
{
    Packet res=pkt;
    // colums of matrix M (appearance in cout output below is transposed)
    __m128 M0,M1,M2;
    const __m128 d0=pkt.d, a0=pkt.a, b0=pkt.b;

    // rows of matrix M
    M0 = _mm_setr_ps(costheta,sintheta,0,0);
    M1 = _mm_setr_ps(-sintheta*cosphi,costheta*cosphi,sinphi,0);
    M2 = _mm_setr_ps(sinphi*sintheta,-sinphi*costheta,cosphi,0);

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

// r0,r1 are the random numbers
/*inline Packet matspin(Packet pkt,float costheta,__m128 cosphi_sinphi)
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

	__m128 zero = _mm_setzero_ps();

	__m128 strig = _mm_addsub_ps(zero,trig);	// (-sin phi) (cos phi) (-sin theta) (cos theta)

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

// r0,r1 are the random numbers
/*inline Packet matspin(Packet pkt,float costheta,__m128 cosphi_sinphi)
{
    Packet res=pkt;

    // SSE naming conventions
    //                                      0   1 2 3
    // Names are from 3..0, eg. 000_cost = cost 0 0 0
    // Note this would be _mm_set_ps(0,0,0,cost)

    // Trying to conform to MSDN docs
    //                          3 2 1 0
    //  _mm_set_ps(a,b,c,d) =   a b c d

    // colums of matrix M (appearance in cout output below is transposed)
    __m128 M0,M1,M2;
    const __m128 d0=pkt.d, a0=pkt.a, b0=pkt.b;

    // _000_cost = 0 0 0 cost
	__m128 _000_cost = _mm_set_ss(costheta);

	__m128 _00_cost_sint = _mm_sqrt_ss(
		_mm_sub_ps(
			_mm_unpacklo_ps(_mm_set_ss(1.0),_000_cost),     // 0    0   cost    1.0
			_mm_mul_ss(_000_cost,_000_cost)));              // 0    0   0       cost^2
	// _00_cost_sint = 0 0 cost sint

	__m128 trig = _mm_shuffle_ps(_00_cost_sint,cosphi_sinphi,_MM_SHUFFLE(1,0,0,1));
    // trig =   sinp    cosp    sint    cost

    // rows of matrix M
//    M0 = _mm_setr_ps(costheta,sintheta,0,0);
//    M1 = _mm_setr_ps(-sintheta*cosphi,costheta*cosphi,sinphi,0);	// 0 sinphi (costheta * cosphi)  (-sintheta * cosphi)
//    M2 = _mm_setr_ps(sinphi*sintheta,-sinphi*costheta,cosphi,0);	// 0 cosphi (-sinphi * costheta) (sinphi * sintheta)

	__m128 zero = _mm_setzero_ps();
	__m128 strig = _mm_addsub_ps(zero,trig);	// (-sin phi) (cos phi) (-sin theta) (cos theta)

	__m128 prods = _mm_mul_ps(
        strig,                                              // -sinp cosp -sint cost
        _mm_shuffle_ps(strig,strig,_MM_SHUFFLE(1,0,2,3)));  // -sint cost cosp  -sinp
    // prods = (sintheta*sinphi) (costheta*cosphi) (-sintheta*cosphi) (-costheta*sinphi)

	__m128 _0_sp_0_cp = _mm_unpackhi_ps(trig,zero);  // 0 sinp 0 cosp

    // The following 3 defs are verified to match M0..M2 in comments above
	M0 = _mm_movelh_ps(trig,zero);                                  // 0 0 sint cost
	M1 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,2,2,1));     // 0 sinp cost*cosp -sint*cosp
	M2 = _mm_shuffle_ps(prods,_0_sp_0_cp,_MM_SHUFFLE(3,0,0,3));     // 0 cosp -cost*sinp sint*sinp

    // d = cos(theta)*d0 - sin(theta)*cos(phi)*a0 + sin(theta)*sin(phi)*b0
    res.d = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(0,0,0,0)));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(0,0,0,0))));
    res.d = _mm_add_ps(res.d,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(0,0,0,0))));

    // a = sin(theta)*d0 +cos(theta)*cos(phi)*a0 - cos(theta)*sin(phi)*b0
    res.a = _mm_mul_ps(d0,_mm_shuffle_ps(M0,M0,_MM_SHUFFLE(1,1,1,1)));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(1,1,1,1))));
    res.a = _mm_add_ps(res.a,_mm_mul_ps(b0,_mm_shuffle_ps(M2,M2,_MM_SHUFFLE(1,1,1,1))));

    // b = sin(phi)*a0 + cos(phi)*b0
    res.b = _mm_mul_ps(a0,_mm_shuffle_ps(M1,M1,_MM_SHUFFLE(2,2,2,2)));
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
 * @param[in] i_uvs		Input unit vectors (8 pairs = 16 floats)
 * @param[out] o_uv		Output values (provides 32 floats: cos/sin x 8)
 */

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

		vcos = _mm256_mul_ps(v_recip_2g,_mm256_sub_ps(v_one_plus_gg,_mm256_mul_ps(t,t)));

		vcos = _mm256_max_ps(_mm256_min_ps(vcos,ones),_mm256_sub_ps(_mm256_setzero_ps(),ones));
	}

	// compute sin x = sqrt(1-cos2 x)
	vsin = _mm256_sqrt_ps(_mm256_sub_ps(ones,_mm256_mul_ps(vcos,vcos)));

	// interleave sine/cosine values
	__m256 vl = _mm256_unpacklo_ps(vcos,vsin);
	__m256 vh = _mm256_unpackhi_ps(vcos,vsin);

	// Interleave to get cos(theta) sin(theta) cos(phi) sin(phi)

	// Note: this permutes the values in i_uv; if i_uv[0..7] are ABCDEFGH then will give ABEFCDGH
	// watch out for this when debugging - may cause confusion but is valid since they're random numbers (order doesn't matter)

	_mm256_store_ps(o,   _mm256_shuffle_ps(vl,uva,_MM_SHUFFLE(1,0,1,0)));
	_mm256_store_ps(o+8, _mm256_shuffle_ps(vl,uva,_MM_SHUFFLE(3,2,3,2)));
	_mm256_store_ps(o+16,_mm256_shuffle_ps(vh,uvb,_MM_SHUFFLE(1,0,1,0)));
	_mm256_store_ps(o+24,_mm256_shuffle_ps(vh,uvb,_MM_SHUFFLE(3,2,3,2)));
}

#endif
