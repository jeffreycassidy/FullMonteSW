#ifndef TETRAMESHBASE_INCLUDED_
#define TETRAMESHBASE_INCLUDED_
#include <iostream>
#include <fstream>
#include <vector>

#include "newgeom.hpp"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <boost/mpl/bool.hpp>

#include <FullMonte/Geometry/TetraMeshBase.hpp>

using namespace std;


class TetraMeshBase {
private:
	template<class Archive>void serialize(Archive& ar,const unsigned int version)
		{	ar & BOOST_SERIALIZATION_NVP(P) & BOOST_SERIALIZATION_NVP(T_p) & BOOST_SERIALIZATION_NVP(T_m); }

protected:
	vector<Point<3,double> >    P;          // point vector
	vector<TetraByPointID>      T_p;        // tetra -> 4 point IDs
	vector<unsigned>			T_m;		// tetra -> material

public:

	virtual void Delete(){ delete this; }

	virtual ~TetraMeshBase(){}

	const vector<Point<3,double>>& 	points() const 	 	{ return P; }
	const vector<TetraByPointID>& 	tetrasByID() const 	{ return T_p; }

	virtual bool checkValid(bool printResults=false) const;

	TetraMeshBase(){};
	TetraMeshBase(const TetraMeshBase& M) = default;
    TetraMeshBase(unsigned Np_,unsigned Nt_) : P(Np_+1,Point<3,double>{0,0,0}),T_p(Nt_+1,TetraByPointID{0,0,0,0}){}

	TetraMeshBase(const vector<Point<3,double> >& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_=vector<unsigned>())
		: P(P_),T_p(T_p_),T_m(T_m_) { if(T_m.size() != T_p.size()) T_m.resize(T_p.size(),0);  }

	unsigned getNp() const { return P.size()-1; };
	unsigned getNt() const { return T_p.size()-1; };

	// Accessors for various point/face constructs
	const Point<3,double>&  getPoint(unsigned id)           const { return P[id]; }
	const TetraByPointID&   getTetraPointIDs(unsigned id)  	const { return T_p[id]; }
    Point<3,double>         getTetraPoint(unsigned IDt,unsigned i) const { return P[T_p[IDt][i]]; }

    double                  getTetraVolume(TetraByPointID IDps) const
        { return abs(scalartriple(P[IDps[0]],P[IDps[1]],P[IDps[2]],P[IDps[3]])/6); }

    double                  getTetraVolume(unsigned IDt) const { return getTetraVolume(T_p[IDt]); }

    const vector<Point<3,double> >& getPoints() const { return P; }
    const vector<TetraByPointID>& getTetrasByPointID() const { return T_p; }

    const vector<unsigned>& getMaterials() const { return T_m; }

    // check if tetra has any point within the region
    array<Point<3,double>,4> tetraPoints(unsigned IDt) const {
    	array<Point<3,double>,4> p;
    	for(unsigned i=0;i<4;++i)
    		p[i]=P[T_p[IDt][i]];
    	return p;
    }


    //template<class Region>TetraMeshBase clipTo(const Region& r) const;

    friend class boost::serialization::access;
};

//TetraMeshBase loadTetraMeshBaseText(const string& fn);
//void saveTetraMeshBaseText(const TetraMeshBase& M,const string& fn);

//template<class Region>TetraMeshBase TetraMeshBase::clipTo(const Region& r) const
//{
//	auto rp = membership_tester(r);
//	vector<unsigned> out_T_m;
//	vector<TetraByPointID> out_T_p;
//
//	out_T_m.push_back(0);
//	out_T_p.push_back(TetraByPointID{0,0,0,0});
//
//	for(unsigned i=0;i<T_p.size();++i)
//	{
//		array<Point<3,double>,4> p=tetraPoints(i);
//		if (all_of(p.begin(),p.end(),rp))
//		{
//			out_T_m.push_back(T_m[i]);
//			out_T_p.push_back(T_p[i]);
//		}
//	}
//
//	return TetraMeshBase(P,out_T_p,out_T_m);
//}

#endif
