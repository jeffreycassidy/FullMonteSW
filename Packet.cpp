#include "Packet.hpp"

bool Packet::checkOrthonormal(float eps) const
{
	return (
			fabs(dot3(a,b)) < eps &&
			fabs(dot3(a,d)) < eps &&
			fabs(dot3(d,b)) < eps &&
			fabs(dot3(a,a)-1.0f) < eps &&
			fabs(dot3(b,b)-1.0f) < eps &&
			fabs(dot3(d,d)-1.0f) < eps);
}

bool Packet::checkOrthonormalVerbose(float eps) const
{
	float dot_ab=dot3(a,b);
	float dot_ad=dot3(a,d);
	float dot_db=dot3(d,b);
	float dot_dd=dot3(d,d);
	float dot_aa=dot3(a,a);
	float dot_bb=dot3(b,b);

	if (fabs(dot_ab) < eps &&
			fabs(dot_ad) < eps &&
			fabs(dot_db) < eps &&
			fabs(dot_aa-1.0f) < eps &&
			fabs(dot_bb-1.0f) < eps &&
			fabs(dot_dd-1.0f) < eps)
		return true;

	cerr << "Orthonormal test failed:" << endl;
	cerr << "  <d,d> = " << dot_dd << endl;
	cerr << "  <d,a> = " << dot_ad << endl;
	cerr << "  <d,b> = " << dot_db << endl;
	cerr << "  <a,a> = " << dot_aa << endl;
	cerr << "  <a,b> = " << dot_ab << endl;
	cerr << "  <b,b> = " << dot_bb << endl;
	return false;
}
