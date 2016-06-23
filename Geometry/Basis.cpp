/*
 * Basis.cpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#include <FullMonteSW/Geometry/Basis.hpp>

Basis::Basis()
{
}

Basis::Basis(const Vector3 b0,const Vector3 b1,const Vector3 b2,const Point3 O)
{
	m_matrix[0] = Vector4{ b0[0], b1[0], b2[0], O[0] };
	m_matrix[1] = Vector4{ b0[1], b1[1], b2[1], O[1] };
	m_matrix[2] = Vector4{ b0[2], b1[2], b2[2], O[2] };
}

Basis Basis::standard()
{
	Basis b;
	for(unsigned i=0;i<3;++i)
	{
		for(unsigned j=0;j<3;++j)
			b.m_matrix[i][j] = i==j;
		b.m_matrix[i][3]=0;
	}
	return b;
}

Point3 Basis::origin() const
{
	return column(3,m_matrix);
}

void Basis::origin(Point3 p)
{
	for(unsigned i=0;i<3;++i)
		m_matrix[i][3] = p[i];
}

UnitVector3 Basis::normal() const
{
	return column(2,m_matrix);
}

UnitVector3 Basis::basis_x() const
{
	return column(0,m_matrix);
}

UnitVector3 Basis::basis_y() const
{
	return column(1,m_matrix);
}

Point3 Basis::invert(Point3 y) const
{
	Point3 x;
	return m_matrix*y;
}

Point3 Basis::invert(Point2 y2) const
{
	Point3 y3{ y2[0], y2[1], 0 };
	return invert(y3);
}


