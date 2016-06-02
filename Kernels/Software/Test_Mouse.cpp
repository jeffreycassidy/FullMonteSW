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

#include <FullMonteSW/Storage/TIMOS/TIMOSAntlrParser.hpp>
#include <FullMonteSW/Storage/TIMOS/TIMOSWriter.hpp>

#include <FullMonteSW/Geometry/Sources/Base.hpp>

#include <FullMonteSW/Kernels/Software/TetraVolumeKernel.hpp>
#include <FullMonteSW/Kernels/Software/TetraSurfaceKernel.hpp>

#include <FullMonteSW/OutputTypes/OutputDataSummarize.hpp>
#include <FullMonteSW/OutputTypes/FluenceConverter.hpp>

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
#include <FullMonteSW/VTK/SparseVectorVTK.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkDataObjectToDataSetFilter.h>
#include <FullMonteSW/VTK/TetraMeshBaseVTK.h>
#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>
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

#ifdef WRAP_VTK
	vtkPoints* P = vtkPoints::New();
	getVTKPoints(M,P);

	vtkCellArray* ca = vtkCellArray::New();
	getVTKTetraCells(M,ca);

	vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
	ug->SetCells(VTK_TETRA,ca);
	ug->SetPoints(P);


	vtkUnsignedShortArray *rgn = vtkUnsignedShortArray::New();
	getVTKTetraRegions(M,rgn);

	const VolumeAbsorbedEnergyMap *V = VK.getResultByType<VolumeAbsorbedEnergyMap>();

	if (!V)
		throw std::logic_error("Failed to get VolumeAbsorbedEnergyMap");

	vtkFloatArray *ve = vtkFloatArray::New();
	getVTKFloatArray(V->get(),ve);

	//ug->GetCellData()->SetScalars(rgn);
	//ug->GetCellData()->SetScalars(ve);

	FluenceConverter FC;
	FC.mesh(&M);
	FC.materials(&mats);

	VolumeFluenceMap phi = FC.convertToFluence(*V);

	vtkFloatArray *vtkPhi = vtkFloatArray::New();
	getVTKFloatArray(phi.get(),vtkPhi);

	ug->GetCellData()->SetScalars(vtkPhi);

	vtkUnstructuredGridWriter *W = vtkUnstructuredGridWriter::New();

	W->SetInputData(ug);
	W->SetFileName("mouse.volume.vtk");
	W->Update();
	W->Delete();
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

#ifdef WRAP_VTK
	vtkPoints* P = vtkPoints::New();
	getVTKPoints(M,P);

	vtkCellArray* ca = vtkCellArray::New();
	vtkIdTypeArray* ids = vtkIdTypeArray::New();

	const SurfaceExitEnergyMap *S = SK.getResultByType<SurfaceExitEnergyMap>();

	if (!S)
		throw std::logic_error("Failed to get SurfaceExitEnergyMap");

	FluenceConverter FC;
	FC.mesh(&M);

	SurfaceFluenceMap phi = FC.convertToFluence(*S);

	const unsigned region = 0U;

	std::vector<unsigned> IDfs = M.getRegionBoundaryTris(region);

	vtkFloatArray *vtkPhi = vtkFloatArray::New();
	vtkPhi->SetNumberOfTuples(IDfs.size());
	ids->SetNumberOfTuples(4*IDfs.size());

	vtkIdType j=0;

	for(const auto IDf : IDfs | boost::adaptors::indexed(0U))
	{
		FaceByPointID IDps = M.getFacePointIDs(IDf.value());
		ids->SetValue(j++,3);

		for(unsigned i=0;i<3;++i)
			ids->SetValue(j++,IDps[i]);

		vtkPhi->SetValue(IDf.index(),phi[IDf.value()]);
	}

	ca->SetCells(IDfs.size(),ids);

	vtkPolyData *pd = vtkPolyData::New();
	pd->SetPolys(ca);
	pd->SetPoints(P);

	pd->GetCellData()->SetScalars(vtkPhi);

	vtkPolyDataWriter *W = vtkPolyDataWriter::New();

	W->SetInputData(pd);
	W->SetFileName("mouse.surface.vtk");
	W->Update();
	W->Delete();
#endif
}
