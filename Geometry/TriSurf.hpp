#pragma once
#include "newgeom.hpp"
#include <boost/serialization/serialization.hpp>

class TriSurf {
public:
	vector<Point<3,double>> P;
	vector<FaceByPointID> 	F;

	template<class Archive>void serialize(Archive& ar,const int version){ ar & P & F; }

	/// Create a TriSurf by copying existing vectors
	TriSurf(const vector<Point<3,double>>& P_,const vector<FaceByPointID>& F_) : P(P_),F(F_){}

	/// Create a TriSurf by move construction
	TriSurf(vector<Point<3,double>>&& P_,vector<FaceByPointID>&& F_) : P(std::move(P_)),F(std::move(F_)){}

	typedef vector<Point<3,double>>::const_iterator point_const_iterator;
	typedef vector<FaceByPointID>::const_iterator	face_id_const_iterator;

	unsigned getNf() const { return F.size(); }		///< Returns the number of faces
	unsigned getNp() const { return P.size(); }		///< Returns the number of points

	void writeASCII(const string& fn) const;
	//void writeMatFile(ostream& os);

	friend boost::serialization::access;
};
