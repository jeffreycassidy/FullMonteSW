/*
 * DirectionalSurface.cpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#include "DirectionalSurface.hpp"
#include <FullMonteHW/Geometry/Filters/FilterBase.hpp>

#include <boost/range/algorithm.hpp>

using namespace std;


DirectionalSurface::DirectionalSurface()
{

}

vector<int> DirectionalSurface::faces() const
{
	return m_faces;
}

vector<unsigned> DirectionalSurface::facesU() const
{
	vector<unsigned> o(m_faces.size());
	boost::transform(m_faces,o.begin(), [](int i){ return std::abs(i); });
	return o;
}

void DirectionalSurface::update()
{
	m_faces.clear();

	for(unsigned IDfu=1;IDfu<m_mesh->getNf(); ++IDfu)
	{
		//			   IDf+ > 0
		// 		Ta   <----------   Tb
		//			 ---------->
		//			   IDf- < 0
		//
		// InternalSurfaceFluenceMap.exit[IDf+] gives amount of fluence leaving Ta
		unsigned Ta=m_mesh->getTetraFromFace(int(IDfu)), Tb=m_mesh->getTetraFromFace(-int(IDfu));

		bool exitFromA;
		if ((exitFromA=(*m_filter)(Ta)) != (*m_filter)(Tb))		// pred(Ta) != pred(Tb) implies face is a boundary
																// pred(Ta) -> Ta included -> exiting
			m_faces.push_back(exitFromA ? IDfu : -IDfu);
	}

}

SurfaceFluenceMap DirectionalSurface::result(FaceDirection dir) const
{
	vector<float> o(m_mesh->getNf()+1,0.0f);

	for(int IDf : m_faces)
	{
		unsigned IDfu = std::abs(IDf);

		if (dir == Bidirectional)
			o[IDfu] = (*m_data)[IDfu].enter + (*m_data)[IDfu].exit;
		else if ((dir == Exit) ^ (IDf < 0))			// IDf < 0 -> invert the face
			o[IDfu] = (*m_data)[IDfu].exit;
		else
			o[IDfu] = (*m_data)[IDfu].enter;
	}

	return SurfaceFluenceMap(SpatialMapBase<float,unsigned>::newFromVector(std::move(o)));
}

InternalSurfaceFluenceMap DirectionalSurface::result() const
{
	vector<InternalSurface<float>> o(m_mesh->getNf()+1,InternalSurface<float>());

	// for each listed face, fetch the data and invert enter/exit if face orientation is negative
	for(int IDf : m_faces)
	{
		unsigned IDfu = std::abs(IDf);

		o[IDfu] = (*m_data)[IDfu];

		if (IDf < 0)
			std::swap(o[IDfu].enter,o[IDfu].exit);
	}

	return InternalSurfaceFluenceMap(SpatialMapBase<InternalSurface<float>,unsigned>::newFromVector(std::move(o)));
}

InternalSurface<float> DirectionalSurface::valueAtFace(int IDf) const
{
	InternalSurface<float> o = (*m_data)[std::abs(IDf)];
	return IDf < 0 ? flip(o) : o;
}


