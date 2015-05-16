#ifndef TETRAMESHBASE_INCLUDED_
#define TETRAMESHBASE_INCLUDED_

#ifndef SWIG
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
#endif


class TetraMeshBase {
private:
	template<class Archive>void serialize(Archive& ar,const unsigned int version)
		{	ar & BOOST_SERIALIZATION_NVP(P) & BOOST_SERIALIZATION_NVP(T_p) & BOOST_SERIALIZATION_NVP(T_m); }

protected:
	std::vector<Point<3,double> >    P;          // point vector
	std::vector<TetraByPointID>      T_p;        // tetra -> 4 point IDs
	std::vector<unsigned>			T_m;		// tetra -> material

public:

	virtual void Delete(){ delete this; }

	virtual ~TetraMeshBase(){}

	const std::vector<Point<3,double>>& 	points() const 	 	{ return P; }
	const std::vector<TetraByPointID>& 	tetrasByID() const 	{ return T_p; }

	virtual bool checkValid(bool printResults=false) const;

	TetraMeshBase(){};
	TetraMeshBase(const TetraMeshBase& M) = default;
    TetraMeshBase(unsigned Np_,unsigned Nt_) : P(Np_+1,Point<3,double>{0,0,0}),T_p(Nt_+1,TetraByPointID{0,0,0,0}){}

	TetraMeshBase(const std::vector<Point<3,double> >& P_,const std::vector<TetraByPointID>& T_p_,const std::vector<unsigned>& T_m_=std::vector<unsigned>())
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

    void remapMaterial(unsigned from,unsigned to);

    const std::vector<Point<3,double> >& getPoints() const { return P; }
    const std::vector<TetraByPointID>& getTetrasByPointID() const { return T_p; }

    const std::vector<unsigned>& getMaterials() const { return T_m; }

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

#endif
