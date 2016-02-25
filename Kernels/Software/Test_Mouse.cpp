/*
 * Test_Mouse.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Test_Mouse

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <FullMonte/Storage/TIMOS/TIMOSAntlrParser.hpp>
#include <FullMonte/Storage/TIMOS/TIMOSWriter.hpp>

#include <FullMonte/Geometry/Sources/Base.hpp>

#include <FullMonte/Kernels/Software/TetraVolumeKernel.hpp>
#include <FullMonte/Kernels/Software/TetraSurfaceKernel.hpp>

#include <FullMonte/OutputTypes/OutputDataSummarize.hpp>
#include <FullMonte/OutputTypes/FluenceConverter.hpp>

#include <string>

using namespace std;

#include "testdata.h"
#include "config.h"

// CMake-configured path to source directory
const string dir(TEST_DATA_PATH "/TIM-OS/mouse");

// headers needed to write out VTK files if so enabled
#ifdef WRAP_VTK
#include <vtkCellTypes.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkDataObjectToDataSetFilter.h>
#include <FullMonte/VTK/vtkFullMonteTetraMeshWrapper.h>
#include <FullMonte/VTK/vtkFullMonteSpatialMapWrapper.h>
#include <vtkMergeDataObjectFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
#include <vtkGeometryFilter.h>
#include <vtkExtractCells.h>
#endif

BOOST_AUTO_TEST_CASE(mouse)
{
	const unsigned Npkt = 1000000;
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
//	TIMOSWriter W("foo");
//	W.write(src);

	TetraVolumeKernel VK(&M);
	VK.rouletteWMin(1e-5);
	VK.materials(mats);
	VK.source(src);
	VK.packetCount(Npkt);
	VK.threadCount(TEST_THREAD_COUNT);

	cout << "Async start" << endl;
	VK.startAsync();

	while(!VK.done())
	{
		cout << "Progress: " << setw(8) << fixed << setprecision(3) << 100.0f*VK.progressFraction() << endl << flush;
		usleep(1000000);
	}

	VK.finishAsync();
	cout << "Finished - results: " << endl;

	for(const auto& r : VK.results())
		cout << "  " << r->typeString() << endl;

	OutputDataSummarize summ(std::cout);

	for(const auto r : VK.results())
		summ(r);

	const MCConservationCountsOutput* cons = VK.getResultByType<MCConservationCountsOutput>();

	// unusual situations that shouldn't happen ever
	BOOST_WARN_EQUAL(cons->w_abnormal, 0.0);
	BOOST_WARN_EQUAL(cons->w_nohit, 0.0);

	// shouldn't happen in this test case (no time gate)
	BOOST_CHECK_EQUAL(cons->w_time, 0.0);

	// conservation relationships - approximate
	BOOST_CHECK_CLOSE(cons->w_roulette,cons->w_die,0.4f);

	// conservation relationships - exact
	BOOST_CHECK_CLOSE(cons->w_launch, double(Npkt), 1e-6);
	BOOST_CHECK_CLOSE(cons->w_launch+cons->w_roulette, 	cons->w_die+cons->w_exit+cons->w_absorb+cons->w_time+cons->w_abnormal+cons->w_nohit, 1e-4);

	BOOST_CHECK_CLOSE(cons->w_exit,double(Npkt)*0.06396,1.0f);
	BOOST_CHECK_CLOSE(cons->w_absorb, double(Npkt)*0.936043,1.0f);



	const MCEventCountsOutput* e = VK.getResultByType<MCEventCountsOutput>();

	BOOST_WARN_EQUAL(e->Nabnormal,0ULL);

	BOOST_CHECK_EQUAL(e->Nlaunch, Npkt);
	BOOST_CHECK_EQUAL(e->Nexit+e->Ndie+e->Ntime+e->Nnohit,Npkt);
	BOOST_CHECK_EQUAL(e->Ntir+e->Nfresnel+e->Nrefr,e->Ninterface);

	BOOST_CHECK_EQUAL(e->Nabsorb,e->Nscatter+e->Ndie);

	// check event counts are within 1% of expected (expect failures for small packet counts due to statistical noise)
	BOOST_CHECK_CLOSE(double(e->Nabsorb), double(Npkt)*375.65, 1.0f);

	BOOST_CHECK_CLOSE(double(e->Nbound), double(Npkt)*166.4, 1.0f);

	BOOST_CHECK_CLOSE(double(e->Ndie), double(Npkt)*0.392, 1.0f);
	BOOST_CHECK_CLOSE(double(e->Nexit), double(Npkt)*0.607959, 1.0f);

	const VolumeAbsorbedEnergyMap *V = VK.getResultByType<VolumeAbsorbedEnergyMap>();

	if (!V)
		throw std::logic_error("Failed to get VolumeAbsorbedEnergyMap");

	FluenceConverter FC;
	FC.mesh(&M);
	FC.materials(&mats);

	VolumeFluenceMap phi = FC.convertToFluence(*V);


#ifdef WRAP_VTK
#endif
}


BOOST_AUTO_TEST_CASE(mouseSurf)
{
	const unsigned Npkt = 1000000;
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

	TetraSurfaceKernel SK(&M);
	SK.rouletteWMin(1e-5);
	SK.materials(mats);
	SK.source(src);
	SK.packetCount(Npkt);
	SK.threadCount(TEST_THREAD_COUNT);

	cout << "Async start" << endl;
	SK.startAsync();

	while(!SK.done())
	{
		cout << "Progress: " << setw(8) << fixed << setprecision(3) << 100.0f*SK.progressFraction() << endl << flush;
		usleep(1000000);
	}

	SK.finishAsync();
	cout << "Finished - results: " << endl;

	for(const auto& r : SK.results())
		cout << "  " << r->typeString() << endl;

	OutputDataSummarize summ(std::cout);

	for(const auto r : SK.results())
		summ(r);

	const MCConservationCountsOutput* cons = SK.getResultByType<MCConservationCountsOutput>();

	// unusual situations that shouldn't happen ever
	BOOST_WARN_EQUAL(cons->w_abnormal, 0.0);
	BOOST_WARN_EQUAL(cons->w_nohit, 0.0);

	// shouldn't happen in this test case (no time gate)
	BOOST_CHECK_EQUAL(cons->w_time, 0.0);

	// conservation relationships - approximate
	BOOST_CHECK_CLOSE(cons->w_roulette,cons->w_die,0.4f);

	// conservation relationships - exact
	BOOST_CHECK_CLOSE(cons->w_launch, double(Npkt), 1e-6);
	BOOST_CHECK_CLOSE(cons->w_launch+cons->w_roulette, 	cons->w_die+cons->w_exit+cons->w_absorb+cons->w_time+cons->w_abnormal+cons->w_nohit, 1e-4);

	BOOST_CHECK_CLOSE(cons->w_exit,double(Npkt)*0.06396,1.0f);
	BOOST_CHECK_CLOSE(cons->w_absorb, double(Npkt)*0.936043,1.0f);



	const MCEventCountsOutput* e = SK.getResultByType<MCEventCountsOutput>();

	BOOST_WARN_EQUAL(e->Nabnormal,0ULL);

	BOOST_CHECK_EQUAL(e->Nlaunch, Npkt);
	BOOST_CHECK_EQUAL(e->Nexit+e->Ndie+e->Ntime+e->Nnohit,Npkt);
	BOOST_CHECK_EQUAL(e->Ntir+e->Nfresnel+e->Nrefr,e->Ninterface);

	BOOST_CHECK_EQUAL(e->Nabsorb,e->Nscatter+e->Ndie);

	// check event counts are within 1% of expected (expect failures for small packet counts due to statistical noise)
	BOOST_CHECK_CLOSE(double(e->Nabsorb), double(Npkt)*375.65, 1.0f);

	BOOST_CHECK_CLOSE(double(e->Nbound), double(Npkt)*166.4, 1.0f);

	BOOST_CHECK_CLOSE(double(e->Ndie), double(Npkt)*0.392, 1.0f);
	BOOST_CHECK_CLOSE(double(e->Nexit), double(Npkt)*0.607959, 1.0f);

	const SurfaceExitEnergyMap *S = SK.getResultByType<SurfaceExitEnergyMap>();

	if (!S)
		throw std::logic_error("Failed to get SurfaceExitEnergyMap");

	FluenceConverter FC;
	FC.mesh(&M);

	SurfaceFluenceMap phiS = FC.convertToFluence(*S);

#ifdef WRAP_VTK

	vtkFullMonteTetraMeshWrapper* vtkM = vtkFullMonteTetraMeshWrapper::New();
		vtkM->mesh(&M);
		vtkM->update();

	/// Build surface rep

	vtkFullMonteSpatialMapWrapper<vtkFloatArray,float,unsigned> *vtkSurfPhi = vtkFullMonteSpatialMapWrapper<vtkFloatArray,float,unsigned>::New();
		vtkSurfPhi->source(phiS.get());
		vtkSurfPhi->array()->SetName("Surface Fluence J/cm2");
		vtkSurfPhi->update();

	// Create fields for cell data
	vtkFieldData* vtkSurfaceField = vtkFieldData::New();
		vtkSurfaceField->AddArray(vtkSurfPhi->array());

	// Create data object holding only the cell data, no geometry
	vtkDataObject* vtkSurfDO = vtkDataObject::New();
		vtkSurfDO->SetFieldData(vtkSurfaceField);

	cout << "VTK faces: " << vtkM->faces()->GetNumberOfCells() << " fluence entries: " << vtkSurfPhi->array()->GetNumberOfTuples() << endl;

	// Merge data object onto geometry
	vtkMergeDataObjectFilter *vtkMergeSurfaceFluence = vtkMergeDataObjectFilter::New();
		vtkMergeSurfaceFluence->SetDataObjectInputData(vtkSurfDO);
		vtkMergeSurfaceFluence->SetInputData(vtkM->faces());
		vtkMergeSurfaceFluence->SetOutputFieldToCellDataField();

	const unsigned region = 0U;

	std::vector<unsigned> IDfs = M.getRegionBoundaryTris(region);

	vtkIdList* surfaceIDs = vtkIdList::New();

	boost::for_each(IDfs, [surfaceIDs](unsigned i){ surfaceIDs->InsertNextId(i); });

	vtkExtractCells *vtkExtractSurface = vtkExtractCells::New();
		vtkExtractSurface->SetInputConnection(vtkMergeSurfaceFluence->GetOutputPort());
		vtkExtractSurface->SetCellList(surfaceIDs);
		vtkExtractSurface->Update();

	vtkGeometryFilter* ug2pd = vtkGeometryFilter::New();
		ug2pd->SetInputConnection(vtkExtractSurface->GetOutputPort());

	vtkPolyDataWriter *W = vtkPolyDataWriter::New();
		W->SetInputConnection(ug2pd->GetOutputPort());
		W->SetFileName("mouse.surface.vtk");
		W->Update();
		W->Delete();
#endif
}
