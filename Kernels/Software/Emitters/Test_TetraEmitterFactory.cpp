/*
 * Test_EmitterFactory.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_MODULE Test_EmitterFactory
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include <FullMonteSW/Storage/TIMOS/TIMOSAntlrParser.hpp>

#include <FullMonteSW/Kernels/Software/RNG_SFMT_AVX.hpp>

#include <FullMonteSW/Kernels/Software/Emitters/TetraMeshEmitterFactory.hpp>

#include <iostream>
#include <iomanip>
#include <array>

#include "../Packet.hpp"

#include "Base.hpp"

#include <FullMonteSW/Storage/TIMOS/TIMOSWriter.hpp>

using namespace std;

#include "TetraMeshEmitterFactory.cpp"


/** Uses the TIM-OS test file "mouse" to test loading of multiple tetra sources */

BOOST_AUTO_TEST_CASE(mouse)
{
	TIMOSAntlrParser R;

	R.setMeshFileName("/Users/jcassidy/src/FullMonteSW/data/mouse.mesh");
	R.setSourceFileName("/Users/jcassidy/src/FullMonteSW/data/mouse.source");

	TetraMesh M = R.mesh();
	Source::Abstract* src = R.sources();

	Emitter::TetraEmitterFactory<RNG_SFMT_AVX> factory(&M);

	TIMOSWriter W("foo");

	src->acceptVisitor(&factory);

	RNG_SFMT_AVX rng;



	auto S = factory.cemitters();

	BOOST_REQUIRE(boost::size(S) != 0);
	BOOST_REQUIRE(S[0] != nullptr);

	cout << "Constructed " << boost::size(S) << " emitters" << endl;

	for(unsigned i=0;i<10;++i)
	{
		LaunchPacket lpkt = S[0]->emit(rng);

		std::array<float,3> pos = lpkt.pos.array();
		std::array<float,3> d = lpkt.dir.d.array();

		cout << std::fixed << std::setprecision(4) <<
				std::setw(9) << pos[0] << ',' << std::setw(9) << pos[1] << ',' << std::setw(9) << pos[2] << "  dir " <<
				std::setw(7) << d[0]   << ',' << std::setw(7) << d[1]   << ',' << std::setw(7) << d[2]   << endl;
	}
}


#include <FullMonteSW/Storage/TIMOS/TIMOSAntlrParser.hpp>
#include <FullMonteSW/Storage/TIMOS/TIMOSWriter.hpp>
#include <FullMonteSW/Storage/VTK/VTKLegacyReader.hpp>

#include <FullMonteSW/Geometry/Placement/PlanePlacement.hpp>

#include <FullMonteSW/Geometry/Sources/Abstract.hpp>

#include <FullMonteSW/Geometry/Filters/TetraFilterByRegion.hpp>
#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>

#include <FullMonteSW/Geometry/Placement/PlanePlacement.hpp>
#include <FullMonteSW/Geometry/Placement/PlanePlacementPencilBeam.hpp>

#include <FullMonteSW/Kernels/Software/RNG_SFMT_AVX.hpp>
#include <FullMonteSW/Kernels/Software/Emitters/TetraMeshEmitterFactory.hpp>

#include <string>

using namespace std;

const string meshfn("/Users/jcassidy/src/Meshing/Release/labeled.mesh.vtk");
const string optpfx("/Users/jcassidy/src/TLT_Knee/MuscleFatSplit");
const string dir("../../");

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

BOOST_AUTO_TEST_CASE(knee)
{
	TIMOSAntlrParser R;
	VTKLegacyReader VR;

	VR.setFileName(meshfn);

	const Point3 tibia {{ 53.7f, -59.7f, 160.6f	}};

	////// Define source positions

	vector<SourceDef> srcs{
		SourceDef{"L1",array<float,3>{{  45.5f, -56.9f, 135.3f }}},
		SourceDef{"L2",array<float,3>{{  42.5f, -64.0f, 135.3f }}},
		SourceDef{"L3",array<float,3>{{  45.9f, -69.4f, 135.3f }}},
		SourceDef{"M1",array<float,3>{{  55.7f, -50.9f, 135.3f }}},
		SourceDef{"M2",array<float,3>{{  60.0f, -54.7f, 135.3f }}},
		SourceDef{"M3",array<float,3>{{  61.4f, -60.1f, 135.3f }}},
		SourceDef{"P",array<float,3> {{  55.7f, -67.5f, 135.5f }}}
	};

	array<float,3> centre{ 53.7f, -59.7f, 135.3f };		// joint center

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


	////// Mesh & materials

	TetraMesh M = VR.mesh();

	cout << "Mesh has " << M.getNt() << " tetras" << endl;

	for(auto& wl : wls)
	{
		cout << "Wavelength: " << wl.label << endl;
		for(const auto src : srcs)
		{
			cout << "  Pos: " << src.label << "  " << src.pos[0] << ' ' << src.pos[1] << ' ' << src.pos[2] <<  endl;

			// set up position
			UnitVector3 dir = normalize(centre-src.pos);
			UnitVector3 lat = normalize(cross(dir,centre-tibia));
			UnitVector3 up = cross(lat,dir);

			wl.placement.basis() = Basis(lat,up,dir,src.pos);
			wl.placement.update();

			// print sources
//			for(const auto* s : wl.placement.sources())
//				cout << *s->source() << endl;

			// grab materials and sources
			Emitter::TetraEmitterFactory<RNG_SFMT_AVX> F(&M);

			wl.placement.source()->acceptVisitor(&F);

//			for(const auto s : wl.placement.sources())
//			{
//				Source::PencilBeam* pb = dynamic_cast<Source::PencilBeam*>(s->source());
//				BOOST_REQUIRE(pb);
//
//				LineQuery LQ;
//					LQ.mesh(&M);
//					LQ.origin(pb->position());
//					LQ.direction(pb->direction());
//
//				for(const auto r : LQ.result())
//					cout << "mat " << r.matID << ' ' << r.f1.p[0] << ' ' << r.f1.p[1] << ' ' << r.f1.p[2] << endl;
//			}


		}
	}
}

