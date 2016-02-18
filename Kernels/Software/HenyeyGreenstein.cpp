/*
 * HenyeyGreenstein.cpp
 *
 *  Created on: Feb 17, 2016
 *      Author: jcassidy
 */


#include "HenyeyGreenstein.hpp"

float henyeyGreensteinDeflection(float g,float x)
{
	float s = 2*x-1.0f;
	float t = (1.0f-g*g)/(1+g*s);
	return 1.0f+g*g-t*t/2.0f/g;
}

void HenyeyGreenstein8f::gParam(float g)
{
	float gg = g*g;
	m_params = _mm_set_ps(0.5f/g,1.0f+gg,1.0f-gg,g);

	float t[4];
	_mm_store_ps(t,m_params);
	m_isotropic = std::abs(g) < 0.01f;
}

float HenyeyGreenstein8f::gParam() const
{
	float f[4];
	_mm_store_ps(f,m_params);
	return f[0];
}
