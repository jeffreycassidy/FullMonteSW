/*
 * DirectionalSurface.cpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#include "DirectionalSurface.hpp"
#include <FullMonteSW/Geometry/TetraMesh.hpp>
#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>

#include <boost/range/algorithm.hpp>

using namespace std;


DirectionalSurface::DirectionalSurface()
{

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

	cout << "DirectionalSurface::update() finished, including " << m_faces.size() << " faces" << endl;

}

OutputData* DirectionalSurface::result() const
{
	AbstractSpatialMap* result;
	double sum=0.0;

	if (m_direction == Directed)
	{
		vector<DirectedSurfaceElement<float>> o(m_mesh->getNf()+1, DirectedSurfaceElement<float>());
		for(int IDf : m_faces)
		{
			unsigned IDfu = std::abs(IDf);
			o[IDfu] = (*m_data)[IDfu];
			if (IDf < 0)
				o[IDfu].flip();
			sum += o[IDfu].enter() + o[IDfu].exit();
		}
		result = new SpatialMap<DirectedSurfaceElement<float>>(std::move(o),
				AbstractSpatialMap::Surface, AbstractSpatialMap::Scalar);
	}
	else
	{
		vector<float> o(m_mesh->getNf()+1,0.0f);
		for(int IDf : m_faces)
		{
			unsigned IDfu = std::abs(IDf);

			if (m_direction == Bidirectional)
				o[IDfu] = (*m_data)[IDfu].bidirectional();
			else if ((m_direction == Exit) ^ (IDf < 0))			// IDf < 0 -> invert the face
				o[IDfu] = (*m_data)[IDfu].exit();
			else
				o[IDfu] = (*m_data)[IDfu].enter();

			sum += o[IDfu];
		}

		//cout << "DirectionalSurface::result() returned a vector of dimension " << o.size() << " totaling " << sum << endl;

		result = new SpatialMap<float>(std::move(o),AbstractSpatialMap::Surface,AbstractSpatialMap::Scalar);
	}

	result->quantity(m_data->quantity());
	result->units(m_data->units());

	return result;
}

DirectedSurfaceElement<float> DirectionalSurface::valueAtFace(int IDf) const
{
	DirectedSurfaceElement<float> o = (*m_data)[std::abs(IDf)];

	if (IDf < 0)
		o.flip();
	return o;
}

void DirectionalSurface::direction(FaceDirection dir)
{
	m_direction = dir;
}

