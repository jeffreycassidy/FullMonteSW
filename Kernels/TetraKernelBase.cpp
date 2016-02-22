/*
 * TetraKernelBase.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */

#include "TetraKernelBase.hpp"
#include <FullMonte/Geometry/SimpleMaterial.hpp>

#include <iomanip>

using namespace std;

TetraKernelBase::Material::Material()
{}

TetraKernelBase::Material::Material(const SimpleMaterial& mat)
{
	// propagation vector along unit physical step: time elapsed increases, dimensionless step decreases by mu_t, and physical step decreases by 1)
	m_prop = _mm_set_ps(0.0f,	mat.n()/c0,	-mat.muT(),	-1.0f);

	// initial propagation vector (1 physical/dimensionless step remaining, 0 time, 0 X)
	m_init = _mm_set_ps(0.0f,	0.0f,		1.0f,		1.0f/mat.muT());

	m_absfrac = mat.absfrac();
	m_scatters = mat.muS() != 0.0f;
	m_n = mat.n();
	m_muT = mat.muT();
}

std::ostream& operator<<(std::ostream& os,const TetraKernelBase::Material& mat)
{
	const auto fmt = os.flags();

	os << fixed << setprecision(3) << "muT=" << mat.muT() << " absfrac=" << mat.absorbedFraction();
	os.flags(fmt);
	return os;
}
