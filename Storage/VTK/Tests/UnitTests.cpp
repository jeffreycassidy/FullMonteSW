/*
 * UnitTests.cpp
 *
 *  Created on: Jan 21, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE UnitTests
#define BOOST_TEST_STATIC_LINK

#include <boost/test/included/unit_test.hpp>

#include "TestConfig.h"

#include "../VTKLegacyReader.hpp"
#include "../VTKLegacyWriter.hpp"

BOOST_AUTO_TEST_CASE ( kneeMeshImport )
{
	VTKLegacyReader R;

	R.setFileName(TEST_FILE("knee.mesh.vtk"));
	TetraMeshBase M = R.mesh();

	cout << "Read OK with Nt=" << M.getNt() << " Np=" << M.getNp() << endl;
	BOOST_CHECKPOINT("Read OK with Nt=" << M.getNt() << " Np=" << M.getNp());

	BOOST_CHECK_EQUAL(M.getNt(),1644829U);		// # in VTK file + 1 (dummy)
	BOOST_CHECK_EQUAL(M.getNp(),267687U);		// # in VTK file + 1 (dummy)

	VTKLegacyWriter W;
	W.writeMeshRegions("output.vtk",M);

	R.setFileName("output.vtk");
	TetraMeshBase MW = R.mesh();

	BOOST_CHECK_EQUAL(MW.getNt(),1644829U);
	BOOST_CHECK_EQUAL(MW.getNp(),267687U);
}
