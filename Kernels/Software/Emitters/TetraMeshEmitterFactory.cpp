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
#include "Tetra.hpp"
#include "Composite.hpp"

#include "TetraMeshEmitterFactory.hpp"

// Source descriptions
#include <FullMonte/Geometry/Sources/Composite.hpp>
#include <FullMonte/Geometry/Sources/PointSource.hpp>
#include <FullMonte/Geometry/Sources/Line.hpp>
#include <FullMonte/Geometry/Sources/Ball.hpp>
#include <FullMonte/Geometry/Sources/Volume.hpp>
#include <FullMonte/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonte/Geometry/Sources/PencilBeam.hpp>
#include <FullMonte/Geometry/Sources/Surface.hpp>

#include <FullMonte/Geometry/RayWalk.hpp>

#include <FullMonte/Geometry/Convenience.hpp>

#include <FullMonte/Geometry/TetraMesh.hpp>

#include "../SSEMath.hpp"

#include <iomanip>

namespace Emitter
{

/** Set up an isotropic point source by locating the tetra enclosing it */

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::PointSource* ps)
{
	unsigned elHint = ps->elementHint(),el=-1U;

	el = m_mesh->findEnclosingTetra(ps->position());

	if (elHint != -1u && el != elHint)
		cout << "WARNING: Element hint provided (" << elHint << ") does not match enclosing tetra found (" << el << ")" << endl;

	Point P(SSE::Vector3(ps->position()));

	auto ips = new PositionDirectionEmitter<RNG,Point,Isotropic<RNG>>(P,Isotropic<RNG>(),el);

	m_emitters.push_back(make_pair(ps->power(),ips));
}



/** Set up a pencil beam by locating the face on which it impinges */

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::PencilBeam* pb)
{
	SSE::Point3 position(pb->position());	// position input
	std::array<float,3> pos;				// the position after advancing to the face
	unsigned tet = -1U, tetHint = pb->elementHint();

	RayWalkIterator it=RayWalkIterator::init(*m_mesh,pb->position(),pb->direction()),it_end;

	if (m_debug)
	{
		array<float,3> pos = pb->position();
		array<float,3> dir = pb->direction();

		cout << "TetraMeshEmitterFactory: starting at " << pos[0] << ' ' << pos[1] << ' ' << pos[2] << " direction " << dir[0] << ' ' << dir[1] << ' ' << dir[2] << endl;
	}

	for(; it != it_end && it->matID == 0; ++it)
	{
		if (m_debug)
			cout << "  Stepping through tet " << setw(9) << it->IDt << " (" << it->dToOrigin << " from origin) " << endl;
		tet=it->IDt;
	}

	if (it == it_end)
		throw std::logic_error("ERROR: TetraEmitterFactory<RNG>::visit(Source::PencilBeam) shows no intersection of ray with mesh face");

	// assign position to the exit point of the last tet with matID=0
	pos = it->f0.p;

	if (tetHint != -1U && tetHint != tet)
		cout << "WARNING: Tetra hint provided (" << tetHint << ") does not match the tetra located by search (" << tet << ")" << endl;
	else if (tetHint == -1U)
		cout << "INFO: No tetra hint provided, found " << tet << " (material " << m_mesh->getMaterial(tet) << ")" << endl;

	if (m_debug)
	{
		cout << "Tetra normals (ID " << tet << ", material " << m_mesh->getMaterial(tet) << ": ";

		::Tetra T = m_mesh->getTetra(tet);
		std::array<float,4> h=T.heights(pos);

		cout << "Heights: ";
		for(unsigned i=0;i<4;++i)
			cout << h[i] << ' ';
		cout << endl;

		if (!T.pointWithin(std::array<double,3>{ pos[0], pos[1], pos[2]}))
			cout << "ERROR! Point is not inside the tetra!" << endl;

	}


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

	Point P{SSE::Vector3(pos)};
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

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::SurfaceTri* st)
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

	throw std::logic_error("TetraEmitterFactory<RNG>::visit - unsupported (SurfaceTri)");
}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Volume* vs)
{
	unsigned IDt=vs->elementID();

	Tetra<RNG> 		T(
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,0))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,1))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,2))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,3))));

	Isotropic<RNG> 	I;
	auto vss = new PositionDirectionEmitter<RNG,Tetra<RNG>,Isotropic<RNG>>(T,I,vs->elementID());

	m_emitters.push_back(make_pair(vs->power(),vss));
}

using namespace std;

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Line* l)
{
	throw std::logic_error("TetraEmitterFactory<RNG>::visit - unsupported (Line)");
}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Composite* c)
{
	for(Source::Base* el : c->elements())
		el->acceptVisitor(this);
}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Surface* s)
{
	throw std::logic_error("TetraEmitterFactory<RNG>::visit - unsupported (Surface)");
}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Base* s)
{
	throw std::logic_error("TetraEmitterFactory<RNG>::visit - unsupported (Source::Base variant called for unknown type)");
}


template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Ball* bs)
{
	vector<unsigned> Ts = m_mesh->tetras_close_to(bs->centre(),bs->radius());
	vector<float> w(Ts.size(), 0.0f);
	float wsum=0.0f;

	for(const auto IDt : Ts)
	{
		w.push_back(m_mesh->getTetraVolume(IDt));
		wsum += w.back();
	}

	for(unsigned i=0;i<Ts.size();++i)
	{
		Source::Volume vs(w[i]/wsum,Ts[i]);
		visit(&vs);
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


