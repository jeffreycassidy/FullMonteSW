/*
 * EmitterFactory.cpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#include "Base.hpp"
#include "Point.hpp"
#include "Directed.hpp"
#include "Isotropic.hpp"
#include "Triangle.hpp"
#include "Tetra.hpp"

#include "EmitterFactory.hpp"

#include <FullMonte/Geometry/Sources/Composite.hpp>
#include <FullMonte/Geometry/Sources/PointSource.hpp>
#include <FullMonte/Geometry/Sources/Line.hpp>
#include <FullMonte/Geometry/Sources/Ball.hpp>
#include <FullMonte/Geometry/Sources/Volume.hpp>
#include <FullMonte/Geometry/Sources/SurfaceTri.hpp>
#include <FullMonte/Geometry/Sources/PencilBeam.hpp>
#include <FullMonte/Geometry/Sources/Surface.hpp>

#include <FullMonte/Geometry/Convenience.hpp>

#include <FullMonte/Geometry/TetraMesh.hpp>


#include "../SSEMath.hpp"
#include "../RandomAVX.hpp"

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

	PointIntersectionResult res;
	int face;

	tie(res,face) = m_mesh->findNextFaceAlongRay(pb->position(), pb->direction());

	if (!res.intersects)
		throw std::logic_error("ERROR: TetraEmitterFactory<RNG>::visit(Source::PencilBeam) shows no intersection of ray with mesh face");

	tet = m_mesh->getTetraFromFace(face);

	boost::copy(res.q, pos.begin());

	if (tetHint != -1U && tetHint != tet)
		cout << "WARNING: Tetra hint provided (" << tetHint << ") does not match the tetra located by search (" << tet << ")" << endl;

	SSE::UnitVector3 d = SSE::UnitVector3::normalize(SSE::Vector3((pb->direction())));
	SSE::UnitVector3 a = SSE::UnitVector3::normalize(cross(d,SSE::UnitVector3::basis<0>()));

	SSE::UnitVector3 b(cross(d,a),SSE::Assert);

	Point P{SSE::Vector3(pos)};
	Directed D(PacketDirection(d,a,b));

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
}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Volume* vs)
{
	unsigned IDt=vs->elementID();

	std::cout << "  Volume element " << vs->elementID() << std::endl;

	Tetra<RNG> 		T(
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,0))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,1))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,2))),
			SSE::Vector3(convertArrayTo<float>(m_mesh->getTetraPoint(IDt,3))));

	Isotropic<RNG> 	I;
	auto vss = new PositionDirectionEmitter<RNG,Tetra<RNG>,Isotropic<RNG>>(T,I);

	m_emitters.push_back(make_pair(vs->power(),vss));
}

using namespace std;

//template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::B)

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Line* l){}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Composite* c)
{
	std::cout << "Composite source" << std::endl;
	for(Source::Base* el : c->elements())
		el->acceptVisitor(this);
}

template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Surface* s){}
template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Base* s){}


template<class RNG>void TetraEmitterFactory<RNG>::visit(Source::Ball* bs)
{
//	vector<unsigned> Ts = m_mesh->tetras_close_to(bs->centre(),bs->radius());
//	vector<float> w(Ts.size(),0.0);
//
//	s.reserve(Ts.size());
//
//	for(unsigned i=0;i<Ts.size();++i)
//	{
//		w[i] = mesh.getTetraVolume(Ts[i]);
//		wsum += w[i];
//	}
//
//	for(unsigned i=0;i<Ts.size();++i)
//	{
//		w[i] = w[i]*bsd->getPower()/wsum;
//		s.push_back(new VolumeSourceEmitter<RNG>(mesh,Ts[i],w[i]));
//		s.back()->setPower(w[i]);
//	}
//	return new SourceMultiEmitter<RNG>(mesh,s.begin(),s.end());
}

template class TetraEmitterFactory<RNG_SFMT_AVX>;

};


