/*
 * SourceFixture.hpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TEST_SOURCEFIXTURE_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TEST_SOURCEFIXTURE_HPP_

#include "../../RNG_SFMT_AVX.hpp"

#ifdef VTK_OUTPUT
#include "VTKPointCloud.hpp"
#include "VTKLineCluster.hpp"
#endif

#include "PacketDirectionFixture.hpp"

/** A test fixture that combines a position with a direction distribution, as well as orthogonality checking for ther packet */

template<class PositionFixture,class DirectionFixture>struct SourceFixture : public PacketDirectionFixture
{
	SourceFixture()
	{
		rng.seed(1);
	}

	void testPacket(LaunchPacket lpkt,unsigned element=-1U)
	{
		testPacketDirection(lpkt.dir);

		directionFixture.testDirection(lpkt.dir.d.array());
		positionFixture.testPosition(lpkt.pos.array());

		history.push_back(lpkt);

		if (element != -1U)
			BOOST_CHECK_EQUAL(lpkt.element,element);
		else if (m_element != -1U)
			BOOST_CHECK_EQUAL(lpkt.element,m_element);
	}

	void write(const std::string fn)
	{
#ifdef VTK_OUTPUT
		VTKPointCloud pcdir;
		VTKPointCloud pcpos;

		VTKLineCluster lc;

		for(const auto& lpkt : history)
		{
			pcdir.add(lpkt.dir.d.array());
			pcpos.add(lpkt.pos.array());

			lc.add(lpkt.pos.array(), lpkt.dir.d.array());
		}

		pcdir.write(fn+".dir.vtk");
		pcpos.write(fn+".pos.vtk");

		lc.write(fn+".lc.vtk");
#endif
	}

	DirectionFixture 	directionFixture;
	PositionFixture 	positionFixture;

	RNG_SFMT_AVX		rng;

	unsigned m_element=-1U;

	std::vector<LaunchPacket>	history;
};

#endif /* KERNELS_SOFTWARE_EMITTERS_TEST_SOURCEFIXTURE_HPP_ */
