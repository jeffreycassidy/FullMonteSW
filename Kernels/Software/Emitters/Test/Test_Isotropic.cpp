/*
 * Test_Isotropic.cpp
 *
 *  Created on: Jan 29, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_Isotropic
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <array>

#include "IsotropicFixture.hpp"
#include "UnitVectorFixture.hpp"
#include "OrthogonalFixture.hpp"
#include "DirectionFixture.hpp"

#include "VTKPointCloud.hpp"

#include <boost/range/algorithm.hpp>

#include <FullMonte/Geometry/Sources/PointSource.hpp>

//#include <FullMonte/Kernels/Software/Emitters/EmitterFactory.hpp>

struct SourceFixture : public DirectionFixture, public IsotropicFixture3
{
	SourceFixture()
	{

	}

	//EmitterFactory efactory;
	//RNG_SFMT_AVX rng;
};

#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

boost::random::mt19937_64 			rng;
boost::random::uniform_01<float>	 u01;



/** Reference implementation that looks OK and passes unit tests */

LaunchPacket launch()
{
	float lambda = u01(rng)*boost::math::constants::two_pi<float>();
	std::array<float,2> azuv{ std::cos(lambda),std::sin(lambda) };

	float sinphi = 2.0f*u01(rng)-1.0f;
	float cosphi = sqrtf(1.0-sinphi*sinphi);


	std::array<float,3> d{ azuv[0]*cosphi , azuv[1]*cosphi,  sinphi };
	std::array<float,3> a{ azuv[0]*sinphi,  azuv[1]*sinphi, -cosphi };
	std::array<float,3> b{ -azuv[1], azuv[0],  0 };

	LaunchPacket lpkt;

	lpkt.dir.d = SSE::UnitVector3(SSE::Vector3(d),SSE::NoCheck);
	lpkt.dir.a = SSE::UnitVector3(SSE::Vector3(a),SSE::NoCheck);
	lpkt.dir.b = SSE::UnitVector3(SSE::Vector3(b),SSE::NoCheck);
	return lpkt;
}

BOOST_FIXTURE_TEST_CASE(isops, SourceFixture)
{
	//std::array<float,3> P{ 1.0, 2.0, 3.0};

//	Sources::PointSource PS(P);
//
//	PS.acceptVisitor(f);
//
//	auto srcs = f.csources();
//
//	BOOST_REQUIRE(boost::size(srcs)==1);

	VTKPointCloud pc;

	for(unsigned i=0;i<100000;++i)
	{
//		LaunchPacket lpkt = srcs[0]->emit(rng);
		LaunchPacket lpkt = launch();

		testDirection(lpkt.dir);

		add(lpkt.dir.d.array());

		pc.add(lpkt.dir.d.array());
	}

	pc.write("iso100k.vtk");
}

template<typename FT,std::size_t D>struct ImplicitPlane
{
	ImplicitPlane(std::array<FT,D> n,FT c) :
		m_n(n),m_c(c){}

	ImplicitPlane(std::array<FT,D> n,std::array<FT,D> p0) :
		m_n(n),m_c(0.0)
	{
		m_c = -operator()(p0);
	}

	/// Evaluates the implicit function (point height over plane)
	FT operator()(std::array<FT,D> p) const
	{
		float dot=-m_c;
		for(unsigned i=0;i<D;++i)
			dot += p[i]*m_n[i];
		return dot;
	}

	/// Flips the orientation of the plane
	void flip()
	{
		for(unsigned i=0;i<D;++i)
			m_n[i] = -m_n[i];
		m_c=-m_c;
	}

	std::array<FT,D> 	m_n;
	FT					m_c;
};

template<typename FT>std::array<FT,3> cross(std::array<FT,3> lhs,std::array<FT,3> rhs)
{
	return std::array<FT,3>{
		lhs[1]*rhs[2] - lhs[2]*rhs[1],
		lhs[2]*rhs[0] - lhs[0]*rhs[2],
		lhs[0]*rhs[1] - lhs[1]*rhs[0]
	};
}


template<typename FT>struct Tetra
{
	typedef std::array<FT,3> Point3;
	typedef std::array<FT,3> Vector3;

	Tetra(Point3 A,Point3 B,Point3 C,Point3 D) :
		m_A(A),m_B(B),m_C(C),m_D(D)
	{
		Vector3 AB{ B[0]-A[0], B[1]-A[1], B[2]-A[2] };
		Vector3 AC{ C[0]-A[0], C[1]-A[1], C[2]-A[2] };
		Vector3 AD{ D[0]-A[0], D[1]-A[1], D[2]-A[2] };
		Vector3 BC{ C[0]-B[0], C[1]-B[1], C[2]-B[2] };
		Vector3 BD{ D[0]-B[0], D[1]-B[1], D[2]-B[2] };

		m_fABC = ImplicitPlane<FT,3>(cross(AB,AC),A);
		if (m_fABC(D) < 0)
			m_fABC.flip();

		m_fABD = ImplicitPlane<FT,3>(cross(AB,AD),A);
		if (m_fABD(C) < 0)
			m_fABD.flip();

		m_fACD = ImplicitPlane<FT,3>(cross(AC,AD),A);
		if (m_fACD(B) < 0)
			m_fACD.flip();

		m_fBCD = ImplicitPlane<FT,3>(cross(BC,BD),B);
		if (m_fBCD(A) < 0)
			m_fBCD.flip();
	}

	// returns the minimum height over any of the four faces (>0 indicates inside the tetra)
	FT operator()(const std::array<FT,3> p)
	{
		std::array<FT,4> heights{
			m_fABC(p),
			m_fABD(p),
			m_fACD(p),
			m_fBCD(p)
		};

		return *boost::min_element(heights);
	}

	Point3 m_A,m_B,m_C,m_D;
	ImplicitPlane<FT,3> m_fABC, m_fABD, m_fACD, m_fBCD;
};

template<typename FT>struct TetraOrigin
{
	std::array<
		std::array<FT,3>,3>	m_matrix;		// Transformation matrix (AB, AC, AD)

	std::array<FT,3>		m_origin;		// Origin point (A)
};
