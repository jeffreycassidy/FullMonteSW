/*
 * BlockRandomDistribution.hpp
 *
 *  Created on: Feb 12, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_BLOCKRANDOMDISTRIBUTION_HPP_
#define KERNELS_SOFTWARE_BLOCKRANDOMDISTRIBUTION_HPP_

#include <mmintrin.h>
#include <smmintrin.h>
#include <emmintrin.h>
#include <xmmintrin.h>

/**
 *
 *
 */

namespace BlockDistribution
{

/// Use block RNG to calculate values of a given distribution, and store them in the designated place
template<class BlockRNG,class Distribution>void calculate(RNG& rng,const Distribution& d,Distribution::result_type* p);

};

class Distribution
{
public:
	typedef	float					result_type;
	static constexpr std::size_t 	OutputBlockSize = 4;

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize = 8;
};


struct FloatVectorBase
{
    static constexpr uint32_t  	exp_float24 = 0x40000000;		// exponent for float in [2,4)
    static constexpr uint32_t  	exp_float 	= 0x3f800000;

    static constexpr uint32_t	float_signmask	= 0x80000000;	// 1b  sign
    static constexpr uint32_t	float_expmask	= 0x7f800000;	// 8b  excess-128 exponent
    static constexpr uint32_t	float_mantmask	= 0x007fffff;	// 23b mantissa (implicit leading 1)

    static const __m256 expmask()
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(float_expmask));
    }

    static const __m256 inv_expmask()
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(~float_expmask));
    }

    static const __m256 signmask()
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(float_signmask));
    }

    static const __m256 inv_signmask()
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(~float_signmask));
    }

    static const __m256 mantmask()
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(float_mantmask));
    }

    static const __m256 inv_mantmask()
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(~float_mantmask));
    }

    /// Returns a vector filled with 2^p
    static const __m256 ldexp(int p)
    {
    	return _mm256_castsi256_ps(_mm256_set1_epi32(((p+128)&0xff) << 23));
    }
};

// [-1,1)
// [0,1)
// 2D unit
// 3D unit
// 2D isospherical (sin/cos phi, sin/cos theta)

class FloatPM1_8_Distribution : public FloatVectorBase
{
public:
	typedef float					result_type;
	static constexpr std::size_t	OutputBlockSize=8;

	typedef uint32_t				input_type;
	static constexpr std::size_t	InputBlockSize=8;

	template<class RNG>void calculate(RNG& rng,float *dst)
	{
		const uint32_t* ip = rng.draw();
		__mm256i i = _mm256_load_epi32(t);

		_mm256_or_ps(
				_mm256_srli_epi32(i,9),
				ldexp(1));

		_mm256_store_ps(dst,o);
	}
};


//static const uint64_t 		exp_double_h = 0x3ff0000000000000ULL;

//inline __m256 RNG_SFMT_AVX::draw_m256f8_u01()
//{
//    __m256i  exp = _mm256_set1_epi32(exp_float);
//    __m256 offs = _mm256_set1_ps(1.0);
//
//    rnd = _emu_mm256_srli_epi32(rnd,9);
//    __m256 rndf = _mm256_or_ps(_mm256_castsi256_ps(rnd),_mm256_castsi256_ps(exp));
//    return _mm256_sub_ps(rndf,offs);
//}



template<class RNG,class Distribution,std::size_t OutputBlocks=1>class BlockDistribution
{
public:
	typedef typename Distribution::result_type	result_type;
	static constexpr std::size_t				outputBlockSize = Distribution::OutputBlockSize;

	const result_type* draw(RNG& rng);

private:

	result_type		m_outputBuffer[OutputBlockSize];
	unsigned		m_pos=0;
	Distribution	m_distribution;
};

template<class RNG,class OutputType>const OutputType* BlockDistribution<RNG,OutputType>::draw(RNG& rng)
{
	if (m_pos == 0)
		BlockDistribution::calculate(rng,m_distribution,m_outputBuffer);

	const OutputType* p = m_outputBuffer+m_pos;

	m_pos = (m_pos+1) % outputBufferSize;

	return p;
}



#endif /* KERNELS_SOFTWARE_BLOCKRANDOMDISTRIBUTION_HPP_ */
