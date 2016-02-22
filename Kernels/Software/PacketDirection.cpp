#include "Packet.hpp"

#include <cmath>
#include <iostream>
#include <tuple>

#include "SSEMath.hpp"

using namespace std;

PacketDirection::PacketDirection(SSE::UnitVector3 d) :
		d(d)
{
	tie(a,b) = SSE::normalsTo(d);
}

/** Checks if the packet is orthonormal to within a tolerance epsilon (eps)
 * @param	eps		Tolerance for checking
 * @returns	True if packet is OK.
 */

bool PacketDirection::checkOrthonormal(float eps) const
{
	return (
			std::abs(float(dot(a,b))) < eps &&
			std::abs(float(dot(a,d))) < eps &&
			std::abs(float(dot(d,b))) < eps &&
			std::abs(float(dot(a,a)))-1.0f < eps &&
			std::abs(float(dot(b,b)))-1.0f < eps &&
			std::abs(float(dot(d,d)))-1.0f < eps);
}


/** Checks if packet is orthonormal and produces output on stderr giving details if not.
 *
 * @param	eps		Tolerance for checking (def 1e-5)
 * @returns True if packet is OK.
 */

bool PacketDirection::checkOrthonormalVerbose(float eps) const
{
	float dot_ab=float(SSE::Vector3::dot(a,b));
	float dot_ad=float(SSE::Vector3::dot(a,d));
	float dot_db=float(SSE::Vector3::dot(d,b));
	float dot_dd=float(SSE::Vector3::dot(d,d));
	float dot_aa=float(SSE::Vector3::dot(a,a));
	float dot_bb=float(SSE::Vector3::dot(b,b));

	if (fabs(dot_ab) < eps &&
			fabs(dot_ad) < eps &&
			fabs(dot_db) < eps &&
			fabs(dot_aa-1.0f) < eps &&
			fabs(dot_bb-1.0f) < eps &&
			fabs(dot_dd-1.0f) < eps)
		return true;

	std::cerr << "Orthonormal test failed:" << std::endl;
	std::cerr << "  <d,d> = " << dot_dd << std::endl;
	std::cerr << "  <d,a> = " << dot_ad << std::endl;
	std::cerr << "  <d,b> = " << dot_db << std::endl;
	std::cerr << "  <a,a> = " << dot_aa << std::endl;
	std::cerr << "  <a,b> = " << dot_ab << std::endl;
	std::cerr << "  <b,b> = " << dot_bb << std::endl;
	return false;
}
