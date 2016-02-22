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


/** The maximum number of materials supported by the RNG (# of Henyey-Greenstein queues to create) */

#ifndef MAX_MATERIALS
#define MAX_MATERIALS 32
#endif


/** Master class for random-number generation. It holds several queues of random numbers that have been
 * generated in a block using vector instructions.
 */

class alignas(32) RNG_SFMT_AVX
{
public:
	RNG_SFMT_AVX(){}

	/// Seed the random-number generator
	void seed(unsigned s)
	{
		URNG::seed(m_sfmt.rng(),s);
	}

	const float* floatU01()		{ return m_floatU01(m_sfmt); 		}		///< U [0,1) float
	const float* floatPM1()		{ return m_floatPM1(m_sfmt);		}		///< U [-1,1) float
	const float* floatExp()		{ return m_expQueue(m_sfmt); 		}		///< Unit exponential float
	const float* hg(unsigned m)	{ return m_hgQueue[m](m_sfmt); 		}		///< cos(theta) sin(theta) cos(psi) sin(psi) HG float

	const float* uvect2D()		{ return m_uvect2D(m_sfmt);			}		///< 2D unit vector (x,y)

	const uint32_t* ui32()		{ return m_ui32Queue(m_sfmt);		}		///< uint32_t random number

	/// Requirements for Boost RNG concept (used for discrete_distribution)
	typedef uint32_t result_type;
	static constexpr result_type min(){ return std::numeric_limits<result_type>::min(); }
	static constexpr result_type max(){ return std::numeric_limits<result_type>::max(); }

	uint32_t operator()(){ return *ui32(); }


	/// Maintain the HG parameters
	void gParamSet(unsigned i,float g){ m_hgQueue[i].distribution().gParam(g); }

private:
	BlockRNGAdaptor<sfmt_t,uint32_t,1024,8,32> 				m_sfmt;						///< The uniform RNG

    BlockRandomDistribution<HenyeyGreenstein8f,2,32>		m_hgQueue[MAX_MATERIALS];	///< Henyey-Greenstein (1 per g value)
    BlockRandomDistribution<FloatUnitExpDistribution,2,32>	m_expQueue;
    BlockRandomDistribution<FloatU01Distribution,1,32>		m_floatU01;
    BlockRandomDistribution<FloatPM1Distribution,1,32>		m_floatPM1;
    BlockRandomDistribution<FloatUVect2Distribution,1,32>	m_uvect2D;
    BlockRandomDistribution<UniformUI32Distribution,1,32>	m_ui32Queue;
};



#endif /* KERNELS_SOFTWARE_RNG_SFMT_AVX_HPP_ */
