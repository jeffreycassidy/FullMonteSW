/*
 * Test_Mouse.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_Mouse

#include <boost/test/unit_test.hpp>

#include <FullMonte/Storage/TIMOS/TIMOSAntlrParser.hpp>
#include <FullMonte/Storage/TIMOS/TIMOSWriter.hpp>

#include <FullMonte/Geometry/Sources/Base.hpp>

#include <FullMonte/Kernels/Software/TetraVolumeKernel.hpp>
#include <FullMonte/Kernels/Software/TetraSurfaceKernel.hpp>

#include <FullMonte/OutputTypes/OutputDataSummarize.hpp>

#include <string>

using namespace std;

const string dir("/Users/jcassidy/src/FullMonteSW/data");

BOOST_AUTO_TEST_CASE(mouse)
{
	TIMOSAntlrParser R;

	// check and make sure it's loaded
	R.setMeshFileName(dir+"/mouse.mesh");
	R.setSourceFileName(dir+"/mouse.source");
	R.setOpticalFileName(dir+"/mouse.opt");

	TetraMesh M = R.mesh();
	vector<SimpleMaterial> mats = R.materials_simple();
	Source::Base* src = R.sources();

	BOOST_CHECK_EQUAL(M.getNt(), 306773U);
	BOOST_CHECK_EQUAL(M.getNf(), 629708U);
	BOOST_CHECK_EQUAL(M.getNp(), 58244U);

	// print out the sources
	TIMOSWriter W("foo");
	W.write(src);

	TetraVolumeKernel VK(&M);
	VK.rouletteWMin(1e-5);
	VK.materials(mats);
	VK.source(src);
	VK.packetCount(1000000);

	cout << "Async start" << endl;
	VK.startAsync();

	while(!VK.done())
	{
		cout << "Progress: " << setw(8) << fixed << setprecision(3) << 100.0f*VK.progressFraction() << endl;
		usleep(1000000);
	}

	VK.finishAsync();
	cout << "Finished - results: " << endl;

	for(const auto& r : VK.results())
		cout << "  " << r->typeString() << endl;

	OutputDataSummarize summ(std::cout);

	for(const auto r : VK.results())
		summ(r);
}


BOOST_AUTO_TEST_CASE(mouseSurface)
{
	TIMOSAntlrParser R;

	// check and make sure it's loaded
	R.setMeshFileName(dir+"/mouse.mesh");
	R.setSourceFileName(dir+"/mouse.source");
	R.setOpticalFileName(dir+"/mouse.opt");

	TetraMesh M = R.mesh();
	vector<SimpleMaterial> mats = R.materials_simple();
	Source::Base* src = R.sources();

	BOOST_CHECK_EQUAL(M.getNt(), 306773U);
	BOOST_CHECK_EQUAL(M.getNf(), 629708U);
	BOOST_CHECK_EQUAL(M.getNp(), 58244U);

	// print out the sources
	TIMOSWriter W("foo");
	W.write(src);

	TetraSurfaceKernel SK(&M);
	SK.rouletteWMin(1e-5);
	SK.materials(mats);
	SK.source(src);
	SK.packetCount(1000000);

	cout << "Async start" << endl;
	SK.startAsync();

	while(!SK.done())
	{
		cout << "Progress: " << setw(8) << fixed << setprecision(3) << 100.0f*SK.progressFraction() << endl;
		usleep(1000000);
	}

	SK.finishAsync();
	cout << "Finished - results: " << endl;

	for(const auto& r : SK.results())
		cout << "  " << r->typeString() << endl;

	OutputDataSummarize summ(std::cout);

	for(const auto r : SK.results())
		summ(r);
}
