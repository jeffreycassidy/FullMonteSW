/*
 * Test_Knee.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include <FullMonte/Storage/TIMOS/TIMOSAntlrParser.hpp>
#include <FullMonte/Storage/TIMOS/TIMOSWriter.hpp>
#include <FullMonte/Storage/VTK/VTKLegacyReader.hpp>

#include <FullMonte/Geometry/Placement/PlanePlacement.hpp>

#include <FullMonte/Geometry/Sources/Base.hpp>

#include <FullMonte/Kernels/Software/TetraSVKernel.hpp>

#include <FullMonte/OutputTypes/OutputDataSummarize.hpp>
#include <FullMonte/OutputTypes/FluenceConverter.hpp>
#include <FullMonte/OutputTypes/DirectionalSurface.hpp>

#include <FullMonte/Geometry/Filters/TetraFilterByRegion.hpp>
#include <FullMonte/Geometry/Filters/TriFilterRegionBounds.hpp>

#include <FullMonte/Geometry/Placement/PlanePlacement.hpp>
#include <FullMonte/Geometry/Placement/PlanePlacementPencilBeam.hpp>


#include <string>

using namespace std;

const string meshfn("/Users/jcassidy/src/Meshing/Release/labeled.mesh.vtk");
const string optpfx("/Users/jcassidy/src/TLT_Knee/MuscleFatSplit");
const string dir("../../");


#include <vtkMergeDataObjectFilter.h>
#include <vtkExtractCells.h>
#include <vtkIdList.h>
#include <vtkCellTypes.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkDataObjectToDataSetFilter.h>
#include <vtkIdList.h>

#include <vtkPolyData.h>
#include <vtkPolyDataWriter.h>

#include <FullMonte/VTK/vtkFullMonteTetraMeshBaseWrapper.h>
#include <FullMonte/VTK/vtkFullMonteSpatialMapWrapper.h>
#include <FullMonte/VTK/vtkFullMonteTetraMeshWrapper.h>

#include <FullMonte/VTK/vtkFullMonteFluenceLineQueryWrapper.h>

#include <vtkGeometryFilter.h>

#include <FullMonte/OutputTypes/FluenceLineQuery.hpp>

#include <FullMonte/Storage/TextFile/TextFileWriter.hpp>

#include <boost/range/counting_range.hpp>
#include <boost/range/adaptor/filtered.hpp>

struct SourceDef
{
	string 			label;
	array<float,3> 	pos;
};

struct WavelengthDef
{
	string					label;
	float					power;
	PlanePlacement			placement;
};

struct MaterialDef
{
	string					label;
	vector<SimpleMaterial>	mats;
};

int main(int argc,char **argv)
{
	const unsigned Npkt = 1000000;		// 1M packets
	float t = 60.0f;					// 60 seconds tx for power scaling

	TIMOSAntlrParser R;
	VTKLegacyReader VR;

	VR.setFileName(meshfn);

	const Point3 tibia { 53.7f, -59.7f, 160.6f	};
	//const Point3 femur { 50.32f, -58.6f, 117.5f };


	////// Define source positions

	vector<SourceDef> srcs{
		SourceDef{"L1",array<float,3>{  45.5f, -56.9f, 135.3f }},
		SourceDef{"L2",array<float,3>{  42.5f, -64.0f, 135.3f }},
		SourceDef{"L3",array<float,3>{  45.9f, -69.4f, 135.3f }},
		SourceDef{"M1",array<float,3>{  55.7f, -50.9f, 135.3f }},
		SourceDef{"M2",array<float,3>{  60.0f, -54.7f, 135.3f }},
		SourceDef{"M3",array<float,3>{  61.4f, -60.1f, 135.3f }},
		SourceDef{"P",array<float,3> {  55.7f, -67.5f, 135.5f }}
	};

	array<float,3> centre{ 53.7f, -59.7f, 135.3f };		// joint center


	for(const auto src : srcs)
	{
		UnitVector3 dir = normalize(centre-src.pos);
		UnitVector3 lat = normalize(cross(dir,centre-tibia));
		UnitVector3 up = cross(lat,dir);

		array<float,3> campos = centre+lat*40.0f;

		cout << "Lat: " << lat[0] << ' ' << lat[1] << ' ' << lat[2] << endl;
		cout << "Cam: " << campos[0] << ' ' << campos[1] << ' ' << campos[2] << endl;
	}


	////// Wavelengths and source patterns

	vector<WavelengthDef> wls{
		WavelengthDef{"660",0.025f,PlanePlacement() },		// 25mW from 4 660nm diodes
		WavelengthDef{"905",0.045f,PlanePlacement() }		// 40mW from 5 905nm diodes
	};


	// the source description - 660nm
	std::vector<Source::PencilBeam> 		pbs660(4);
	std::vector<PlanePlacementPencilBeam>	mediators660(pbs660.size());

	float r660 = sqrt(0.5f)*1.75f;
	std::vector<Point2>						pbsPos660{
		Point2{ r660, r660 },
		Point2{-r660, r660 },
		Point2{-r660,-r660 },
		Point2{ r660,-r660 }
	};

	for(unsigned i=0;i<pbs660.size();++i)
	{
		mediators660[i].placement(&wls[0].placement);
		mediators660[i].source(&pbs660[i]);
		mediators660[i].planePosition(pbsPos660[i]);
		mediators660[i].update();
	}


	// source pattern - 905nm

	std::vector<Source::PencilBeam>			pbs905(5);
	std::vector<PlanePlacementPencilBeam> 	mediators905(pbs905.size());

	std::vector<Point2>						pbsPos905{
		Point2{  0.0f ,  0.0f },
		Point2{  1.75f,  0.0f },
		Point2{ -1.75f,  0.0f },
		Point2{  0.0f , -1.75f},
		Point2{  0.0f ,  1.75f}
	};


	for(unsigned i=0;i<pbs905.size();++i)
	{
		mediators905[i].placement(&wls[1].placement);
		mediators905[i].source(&pbs905[i]);
		mediators905[i].planePosition(pbsPos905[i]);
		mediators905[i].update();
	}


	WavelengthDef& wl = wls[0];

	////// Materials - define names

	vector<string> matstrs{
		"Light",
		"Med",
		"Dark",
		"XDark"
	};

	string matstr=matstrs[0];


	////// Mesh & materials

	TetraMesh M = VR.mesh();



	////// The Monte Carlo kernel

	TetraSVKernel SV(&M);
	SV.rouletteWMin(1e-5);

	SV.maxSteps(500000);
	SV.packetCount(Npkt);
	SV.threadCount(8);




	////// Fluence conversion

	FluenceConverter FC;
		FC.mesh(&M);
		FC.joulesPerOutputEnergyUnit(1.0f);
		FC.cmPerOutputLengthUnit(1.0f);




	////// Create VTK wrapper for mesh

	vtkFullMonteTetraMeshWrapper *vtkM = vtkFullMonteTetraMeshWrapper::New();
	vtkM->mesh(&M);




	////// Create output pipeline for surface fluence

	vtkFullMonteSpatialMapWrapper<vtkFloatArray,float,unsigned> *vtkSurfPhi = vtkFullMonteSpatialMapWrapper<vtkFloatArray,float,unsigned>::New();
		vtkSurfPhi->array()->SetName("Surface Fluence J/cm2");

	// Create fields for cell data
	vtkFieldData* vtkSurfField = vtkFieldData::New();
	vtkSurfField->AddArray(vtkSurfPhi->array());

	// Create data object holding only the cell data, no geometry
	vtkDataObject* vtkSurfDO = vtkDataObject::New();
	vtkSurfDO->SetFieldData(vtkSurfField);

	// Merge data object onto geometry
	vtkMergeDataObjectFilter *vtkMergeSurfaceFluence = vtkMergeDataObjectFilter::New();
	vtkMergeSurfaceFluence->SetDataObjectInputData(vtkSurfDO);
	vtkMergeSurfaceFluence->SetInputData(vtkM->faces());
	vtkMergeSurfaceFluence->SetOutputFieldToCellDataField();

	TriFilterRegionBounds TF(&M);
	TF.includeRegion(5,true);
	M.setFacesForFluenceCounting(&TF);

	vtkIdList* surfaceIDs = vtkIdList::New();

	boost::for_each(
			boost::counting_range<int>(0,M.getNf()+1) | boost::adaptors::filtered(TF),
			[surfaceIDs](int i){ surfaceIDs->InsertNextId(i); });

	vtkExtractCells *vtkExtractSurface = vtkExtractCells::New();
	vtkExtractSurface->SetInputConnection(vtkMergeSurfaceFluence->GetOutputPort());
	vtkExtractSurface->SetCellList(surfaceIDs);
	vtkExtractSurface->Update();

	vtkGeometryFilter* ug2pd = vtkGeometryFilter::New();
	ug2pd->SetInputConnection(vtkExtractSurface->GetOutputPort());

	vtkPolyDataWriter *vtkSurfWriter = vtkPolyDataWriter::New();
	vtkSurfWriter->SetInputConnection(ug2pd->GetOutputPort());



	FC.scaleTotalEmittedTo(wl.power * t);

	R.setOpticalFileName(dir+"MuscleFatSplit"+wl.label+matstr+".opt");
	vector<SimpleMaterial> mats = R.materials_simple();

	SV.materials(mats);
	FC.materials(&mats);

	auto src = srcs[0];

	// set up position
	UnitVector3 dir = normalize(centre-src.pos);
	UnitVector3 lat = normalize(cross(dir,centre-tibia));
	UnitVector3 up = cross(lat,dir);

	wl.placement.basis() = Basis(lat,up,dir,src.pos);
	wl.placement.update();

	// grab materials and sources
	SV.source(wl.placement.source());

	// print sources
	cout << "Sources: " << endl;
	for(const auto* s : wl.placement.sources())
		cout << *s->source() << endl;
	cout << endl;

	cout << "Async start" << endl;
	SV.startAsync();

	while(!SV.done())
	{
		cout << "Progress: " << setw(8) << fixed << setprecision(3) << 100.0f*SV.progressFraction() << endl << flush;
		usleep(1000000);
	}

	SV.finishAsync();
	cout << "Finished - results: " << endl;

	for(const auto& r : SV.results())
		cout << "  " << r->typeString() << endl;

	OutputDataSummarize summ(std::cout);

	for(const auto r : SV.results())
		summ(r);


	////// Convert surface exit map to fluence
	const InternalSurfaceEnergyMap *S = SV.getResultByType<InternalSurfaceEnergyMap>();

	if (!S)
		throw std::logic_error("Failed to get SurfaceExitEnergyMap");
	else
		cout << "Internal surface map: dim=" << (*S)->dim() << " nnz=" << (*S)->nnz() << endl;


	InternalSurfaceFluenceMap phiDS = FC.convertToFluence(*S);

	// include all regions except 0 (exterior)
	TetraFilterByRegion rgnF;
		rgnF.mesh(&M);
		rgnF.includeAll();
		rgnF.include(0,false);

	DirectionalSurface DS;
		DS.mesh(&M);
		DS.tetraFilter(&rgnF);
		DS.data(&phiDS);
		DS.update();

		{
			auto phiS = DS.result(Exit);

			vtkSurfPhi->source(phiS.get());
			vtkSurfPhi->update();

			vtkSurfWriter->SetFileName("exit_tissue.vtk");
			vtkSurfWriter->Update();
		}

		{
			auto phiS = DS.result(Enter);

			vtkSurfPhi->source(phiS.get());
			vtkSurfPhi->update();

			vtkSurfWriter->SetFileName("enter_tissue.vtk");
			vtkSurfWriter->Update();
		}

		{
			auto phiS = DS.result(Bidirectional);

			vtkSurfPhi->source(phiS.get());
			vtkSurfPhi->update();

			vtkSurfWriter->SetFileName("bidir_tissue.vtk");
			vtkSurfWriter->Update();
		}

}
