/*
 * RNG_SFMT_AVX.hpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_RNG_SFMT_AVX_HPP_
#define KERNELS_SOFTWARE_RNG_SFMT_AVX_HPP_

#include "BlockRandomDistribution.hpp"
#include "SFMTWrapper.hpp"
#include "BlockRNGAdaptor.hpp"

#include "HenyeyGreenstein.hpp"
#include "FloatUnitExpDistribution.hpp"
#include "FloatU01Distribution.hpp"
#include "FloatPM1Distribution.hpp"
#include "UniformUI32Distribution.hpp"
#include "FloatUVect2Distribution.hpp"

#ifndef MAX_MATERIALS
#define MAX_MATERIALS 32
#endif

class alignas(32) RNG_SFMT_AVX
{
public:
	RNG_SFMT_AVX(){}

	void seed(unsigned s)
	{
		URNG::seed(m_sfmt.rng(),s);
	}

	const float* floatU01()		{ return m_floatU01(m_sfmt); 		}
	const float* floatPM1()		{ return m_floatPM1(m_sfmt);		}
	const float* floatExp()		{ return m_expQueue(m_sfmt); 		}
	const float* hg(unsigned m)	{ return m_hgQueue[m](m_sfmt); 		}

	const float* uvect2D()		{ return m_uvect2D(m_sfmt);			}

	typedef uint32_t result_type;
	static constexpr result_type min(){ return std::numeric_limits<result_type>::min(); }
	static constexpr result_type max(){ return std::numeric_limits<result_type>::max(); }

	uint32_t operator()(){ return *m_ui32Queue(m_sfmt); }

	void gParamSet(unsigned i,float g){ m_hgQueue[i].distribution().gParam(g); }

private:
	BlockRNGAdaptor<sfmt_t,uint32_t,1024,8,32> 				m_sfmt;

    BlockRandomDistribution<HenyeyGreenstein8f,2,32>		m_hgQueue[MAX_MATERIALS];
    BlockRandomDistribution<FloatUnitExpDistribution,2,32>	m_expQueue;
    BlockRandomDistribution<FloatU01Distribution,1,32>		m_floatU01;
    BlockRandomDistribution<FloatPM1Distribution,1,32>		m_floatPM1;
    BlockRandomDistribution<FloatUVect2Distribution,1,32>	m_uvect2D;
    BlockRandomDistribution<UniformUI32Distribution,1,32>	m_ui32Queue;
};



#endif /* KERNELS_SOFTWARE_RNG_SFMT_AVX_HPP_ */
