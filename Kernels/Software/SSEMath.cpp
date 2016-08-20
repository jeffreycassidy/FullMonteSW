/*
 * SSEMath.cpp
 *
 *  Created on: Jan 31, 2016
 *      Author: jcassidy
 */

#include <utility>
#include <cmath>
#include <array>
#include "SSEMath.hpp"

using namespace std;

namespace SSE
{

class SSEKernel
{
public:
	template<std::size_t D>class Vector;
	template<std::size_t D>class Point;
	template<std::size_t D>class UnitVector;

	using Vector3 = Vector<3>;
	using Vector2 = Vector<2>;

	using UnitVector3 = UnitVector<3>;
	using UnitVector2 = UnitVector<2>;
};

pair<SSE::UnitVector3,SSE::UnitVector3> normalsTo(SSE::UnitVector3 v)
{
	const array<float,3> d = v.array();
	array<float,4> aa;

	// create normal by zeroing smallest element, transposing remaining two elements, and normalizing
	// eg. [dx dy dz] => [-dy dx 0] if |dz| < |dy|,|dx|

	const unsigned m = v.indexOfSmallestElement();

	const unsigned i=(m+1)%3,j=(m+2)%3;
	float k = 1.0/std::sqrt(d[i]*d[i] + d[j]*d[j]);

	aa[m] = 0.0f;
	aa[i] = d[j]*k;
	aa[j] = -d[i]*k;

	SSE::UnitVector3 a(SSE::Vector3(_mm_load_ps(aa.data())),SSE::Assert);

	return make_pair(
			a,
			SSE::UnitVector3(cross(v,a),SSE::Assert));
}



};
