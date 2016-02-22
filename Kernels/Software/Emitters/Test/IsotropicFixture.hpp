/*
 * IsotropicFixture.hpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_ISOTROPICFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_ISOTROPICFIXTURE_HPP_

#include <array>
#include <cmath>

#include <boost/test/floating_point_comparison.hpp>

#include <boost/math/constants/constants.hpp>

template<typename FT>struct Polar
{
	FT r;		///< Radius
	FT phi;		///< Elevation
	FT lambda;	///< Azimuth
};

/** Checks that a source has an isotropic emission pattern
 * TODO: Need to check the accumulated statistics when it's done! */

struct IsotropicFixture
{
	IsotropicFixture()
	{
	}
	~IsotropicFixture()
	{
	}

	template<typename FT>Polar<FT> cartesianToPolar(std::array<FT,3> pc)
	{
		Polar<FT> pp;

		pp.r = std::sqrt(pc[0]*pc[0] + pc[1]*pc[1] + pc[2]*pc[2]);					// asin returns principal [-pi/2,pi/2)
		pp.phi = std::acos(std::sqrt(pc[0]*pc[0] + pc[1]*pc[1]) / pp.r) - halfpi;	// acos returns principal [0,pi)
		pp.lambda = std::atan2(pc[0],pc[1]);

		return pp;
	}

	template<typename FT>std::array<FT,3> polarToCartesian(Polar<FT> pp)
	{
		std::array<FT,3> pc;

		FT cosphi = std::cos(pp.phi);

		pc.x = pp.r*cosphi*std::cos(pp.lambda);
		pc.y = pp.r*cosphi*std::sin(pp.lambda);
		pc.z = pp.r*std::sin(pp.phi);

		return pc;
	}

	template<typename FT>void testDirection(std::array<FT,3> dir)
	{
		//Polar<FT> pc = cartesianToPolar(dir);
	}

	template<typename FT>std::array<unsigned,2> bin2ForPolar(Polar<FT> pc)
	{
		unsigned binLambda = (pc.lambda/pi+0.5)/FT(m_nLambdaBins);		// even spacing in lambda
		unsigned binPhi = (std::sin(pc.phi)+1.0)/FT(m_nPhiBins);		// even spacing in sin(phi)

		assert(binLambda < m_nLambdaBins);								// check no bin overflow
		assert(binPhi < m_nPhiBins);

		return std::array<unsigned,2>{ binLambda, binPhi };
	}

	static constexpr double pi 		= boost::math::constants::pi<double>();
	static constexpr double halfpi 	= boost::math::constants::pi<double>()*0.5;

	unsigned m_nPhiBins=10;
	unsigned m_nLambdaBins=10;
};



#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_ISOTROPICFIXTURE_HPP_ */
