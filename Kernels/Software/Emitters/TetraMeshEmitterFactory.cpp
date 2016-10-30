/*
 * EmitterFactory.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */


// Emitter classes
#include "Base.hpp"
#include "Point.hpp"
#include "Directed.hpp"
#include "Isotropic.hpp"
#include "Triangle.hpp"
#include "Line.hpp"
#include "Tetra.hpp"
#include "Composite.hpp"

#include "TetraMeshEmitterFactory.hpp"

// Source descriptions
#include <FullMonteSW/Geometry/Sources/Composite.hpp>
#include <FullMonteSW/Geometry/Sources/Point.hpp>
#include <FullMonteSW/Geometry/Sources/Line.hpp>
#include <FullMonteSW/Geometry/Sources/Ball.hpp>
#include <FullMonteSW/Geometry/Sources/Volume.hpp>
#include <FullMonteSW/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonteSW/Geometry/Sources/PencilBeam.hpp>
#include <FullMonteSW/Geometry/Sources/Surface.hpp>

#include <FullMonteSW/Geometry/Filters/TriFilterRegionBounds.hpp>
#include <FullMonteSW/Geometry/Filters/TetrasNearPoint.hpp>
#include <FullMonteSW/Geometry/RayWalk.hpp>

#include <FullMonteSW/Geometry/Convenience.hpp>

#include <FullMonteSW/Geometry/TetraMesh.hpp>

#include "../SSEMath.hpp"

#include <iomanip>

#include <FullMonteSW/Kernels/Software/RNG_SFMT_AVX.hpp>
#include "RandomInPlane.hpp"

namespace Emitter
{

/** Set up an isotropic point source by locating the tetra enclosing it */

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Point* ps)
{
	unsigned elHint = ps->elementHint(),el=-1U;

	el = m_mesh->findEnclosingTetra(ps->position());

	if (elHint != -1u && el != elHint)
		cout << "WARNING: Element hint provided (" << elHint << ") does not match enclosing tetra found (" << el << ")" << endl;

	array<float,3> pos = ps->position();

	cout << "  INFO: Point source at " << pos[0] << ',' << pos[1] << ',' << pos[2] << endl << " in tetra " << el << endl;

	Point P(el,SSE::Point3(ps->position()));

	auto ips = new PositionDirectionEmitter<RNG,Point,Isotropic<RNG>>(P,Isotropic<RNG>(),el);

	m_emitters.push_back(make_pair(ps->power(),ips));
}



/** Set up a pencil beam by locating the face on which it impinges */

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::PencilBeam* pb)
{
	SSE::Point3 position(pb->position());	// position input
	std::array<float,3> pos;				// the position after advancing to the face
	unsigned tet = -1U, tetHint = pb->elementHint();

//	RayWalkIterator it=RayWalkIterator::init(*m_mesh,pb->position(),pb->direction()),it_end;
//
//	if (m_debug)
//	{
//		pos = pb->position();
//		array<float,3> dir = pb->direction();
//
//		cout << "TetraMeshEmitterFactory: starting at " << pos[0] << ' ' << pos[1] << ' ' << pos[2] << " direction " << dir[0] << ' ' << dir[1] << ' ' << dir[2] << endl;
//	}
//
//	for(; it != it_end && it->matID == 0; ++it)
//	{
//		if (m_debug)
//			cout << "  Stepping through tet " << setw(9) << it->IDt << " (" << it->dToOrigin << " from origin) " << endl;
//		tet=it->IDt;
//	}
//
//	if (it == it_end)
//		throw std::logic_error("ERROR: TetraEmitterFactory<RNG>::visit(Source::PencilBeam) shows no intersection of ray with mesh face");
//
//	// assign position to the exit point of the last tet with matID=0
//	pos = it->f0.p;

	TriFilterRegionBounds TF(m_mesh);
	TF.includeRegion(0,true);			// TODO: remove hard-coded value here
	TF.includeRegion(18,false);			// TODO: remove hard-coded value here
	TF.bidirectional(false);			// include face only if going from mat 0 into mat !0

#warning "Hard-coded region bounds for face-intersection calculation in TetraMeshEmitterFactory for PencilBeam"

	RTIntersection res = m_mesh->findSurfaceFace(pb->position(), pb->direction(), &TF);

	tet = res.IDt;
	pos = res.q;
	m_debug=true;

	if (tetHint != -1U && tetHint != tet)
		cout << "WARNING: Tetra hint provided (" << tetHint << ") does not match the tetra located by search (" << tet << ")" << endl;
	else if (tetHint == -1U && m_debug)
		cout << "INFO: No tetra hint provided, found " << tet << " (material " << m_mesh->getMaterial(tet) << ")" << endl;

	if (m_debug)
	{
		cout << "INFO: Created pencil beam emitter impinging on face " << res.IDf << " into tetra " << res.IDt << " with material " <<
			m_mesh->getMaterial(res.IDt) << " at position " << res.q[0] << ',' << res.q[1] << ',' << res.q[2] << endl;

		unsigned IDt_opposite = m_mesh->getTetraFromFace(-res.IDf);

		cout << "INFO:   Opposite face " << -res.IDf << " impinges tet " << IDt_opposite << " material " << m_mesh->getMaterial(IDt_opposite) << endl;

	}


	::Tetra T = m_mesh->getTetra(tet);
	bool showTetStats=m_debug;

	if (!T.pointWithin(to_m128(pos),m_tetraInteriorEpsilon))
	{
		showTetStats=true;
		cout << "ERROR! Point is not inside the tetra!" << endl;
	}
//
//	if (showTetStats)
//		{
//			cout << "Tetra normals (ID " << tet << ", material " << m_mesh->getMaterial(tet) << ": ";
//
//			std::array<float,4> h = to_array<float,4>(T.heights(to_m128(pos)));
//
//			cout << "Heights: ";
//			for(unsigned i=0;i<4;++i)
//				cout << h[i] << ' ';
//			cout << endl;
//		}



	// compute normals a,b for d
	SSE::UnitVector3 d = SSE::UnitVector3::normalize(SSE::Vector3((pb->direction())));

	SSE::UnitVector3 a;
	if (std::abs(a[0]) < std::abs(a[1]))
	{
		if (std::abs(a[0]) < std::abs(a[2]))
			a = SSE::UnitVector3::normalize(cross(d,SSE::UnitVector3::basis<0>()));
		else
			a = SSE::UnitVector3::normalize(cross(d,SSE::UnitVector3::basis<2>()));
	}
	else if (std::abs(a[1]) < std::abs(a[2]))
		a = SSE::UnitVector3::normalize(cross(d,SSE::UnitVector3::basis<1>()));
	else
		a = SSE::UnitVector3::normalize(cross(d,SSE::UnitVector3::basis<2>()));

	SSE::UnitVector3 b(cross(d,a),SSE::Assert);

	Point P{tet,SSE::Vector3(pos)};
	Directed D(PacketDirection(d,a,b));

	if (m_debug)
	{
		std::array<float,3> df = d.array();
		std::array<float,3> af = a.array();
		std::array<float,3> bf = b.array();

		cout << "w=" << pb->power() << " Position: " << pos[0] << ' ' << pos[1] << ' ' << pos[2] << "  direction: " << df[0] << ' ' << df[1] << ' ' << df[2] << endl;
		cout << "a=" << af[0] << ' ' << af[1] << ' ' << af[2] << "  b: " << bf[0] << ' ' << bf[1] << ' ' << bf[2] << endl;
	}

	auto pbs = new PositionDirectionEmitter<RNG,Point,Directed>(P,D,tet);
	m_emitters.push_back(make_pair(pb->power(),pbs));
}

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::SurfaceTri* st)
{
//	// find face in TetraMesh
//	IDf = m.getFaceID(f);
//
//	if(force_boundary)
//	{
//		unsigned tetraDown = m.getTetraFromFace(-IDf), tetraUp = m.getTetraFromFace(IDf);
//		if (tetraUp){
//			IDt=tetraUp;
//			if (tetraDown)
//				cerr << "Surprise: source is not on a boundary" << endl;
//			else
//				IDf = -IDf;
//		}
//		else if (tetraDown)
//			IDt=tetraDown;
//	}
//	else
//		IDt = m.getTetraFromFace(IDf);


//	Triangle<RNG> T(A,B,C);
//	Directed D(dir);
//
//	auto sts = new PositionDirectionEmitter<RNG,Triangle<RNG>,Directed>(T,D);
//
//	m_emitters.push_back(make_pair(st->power(),sts));

	throw std::logic_error("TetraEmitterFactory<RNG>::doVisit - unsupported (SurfaceTri)");
}

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Volume* vs)
{
	unsigned IDt=vs->elementID();

	Tetra<RNG> 		T(
			IDt,
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,0))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,1))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,2))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,3))));

	Isotropic<RNG> 	I;
	auto vss = new PositionDirectionEmitter<RNG,Tetra<RNG>,Isotropic<RNG>>(T,I);

	m_emitters.push_back(make_pair(vs->power(),vss));
}

using namespace std;

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Line* l)
{
	Emitter::EmitterBase<RNG>* ls=nullptr;

	// each segment is a (length, IDt) pair specifying the tetras that make up the line source
	std::vector<std::pair<float,unsigned>> segments;

	RayWalkIterator it=RayWalkIterator::init(
			*m_mesh,
			l->endpoint(0),
			normalize(l->endpoint(1)-l->endpoint(0)));

	RayWalkIterator itEnd = RayWalkIterator::endAt(l->length());

	for(; it != itEnd; ++it)
		segments.emplace_back(it->dToOrigin+it->lSeg,it->IDt);

	cout << "Created a line source with " << segments.size() << " segments" << endl;

	for(const auto& s : segments)
		cout << "  tet " << s.second << " l up to " << s.first << endl;

	Emitter::Line<RNG> P(
			SSE::Point3(l->endpoint(0)),
			SSE::Point3(l->endpoint(1)),
			segments);

	switch(l->pattern())
	{
	case Source::Line::Isotropic:
		ls = new PositionDirectionEmitter<RNG,Line<RNG>,Isotropic<RNG>>(P,Isotropic<RNG>());
		break;
	case Source::Line::Normal:
		ls = new PositionDirectionEmitter<RNG,Line<RNG>,RandomInPlane<RNG>>(P,RandomInPlane<RNG>(P.displacement()));
		break;
	default:
		throw std::logic_error("TetraEmitterFactory<RNG>::doVisit(Source::Line*) unsupported emission pattern");
	}

	m_emitters.push_back(make_pair(l->power(),ls));
}

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Composite* c)
{
	for(Source::Abstract* el : c->elements())
		el->acceptVisitor(this);
}

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Surface* s)
{
	throw std::logic_error("TetraEmitterFactory<RNG>::doVisit - unsupported (Surface)");
}

template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Abstract* s)
{
	throw std::logic_error("TetraEmitterFactory<RNG>::doVisit - unsupported (Source::Base variant called for unknown type)");
}


template<class RNG>void TetraEmitterFactory<RNG>::doVisit(Source::Ball* bs)
{
	TetrasNearPoint TP;

	TP.mesh(m_mesh);
	TP.centre(bs->centre());
	TP.radius(bs->radius());

	vector<std::pair<unsigned,float>> w;
	float wsum=0.0f;

	for(const auto IDt : m_mesh->tetras())
	{
		if (TP(IDt.value()))
		{
			w.emplace_back(IDt.value(),get(volume,*m_mesh,IDt));
			wsum += w.back().second;
		}
	}

	for(const auto p : w)
	{
		Source::Volume vs(p.second/wsum,p.first);
		doVisit(&vs);
	}
}

template<class RNG>Emitter::EmitterBase<RNG>* TetraEmitterFactory<RNG>::emitter() const
{
	EmitterBase<RNG>* e=nullptr;

	if (m_emitters.size() == 0)
		throw std::logic_error("TetraEmitterFactory<RNG>::source() - no sources!");
	else if (m_emitters.size() == 1)
		e = m_emitters.front().second;
	else
		e = new Emitter::Composite<RNG>(m_emitters);

	return e;
}

};



