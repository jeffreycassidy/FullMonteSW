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

#include <cmath>


class TetraMeshBase
{
public:

	TetraMeshBase(){};

	TetraMeshBase(TetraMeshBase&& M) : P(std::move(M.P)),T_p(std::move(M.T_p)),T_m(std::move(M.T_m)){}
	TetraMeshBase(const TetraMeshBase& M) = default;
	TetraMeshBase& operator=(TetraMeshBase&&) = default;
	TetraMeshBase& operator=(const TetraMeshBase&) = default;

    TetraMeshBase(unsigned Np_,unsigned Nt_) : P(Np_+1,Point<3,double>{0,0,0}),T_p(Nt_+1,TetraByPointID{0,0,0,0}){}

	TetraMeshBase(const std::vector<Point<3,double> >& P_,const std::vector<TetraByPointID>& T_p_,const std::vector<unsigned>& T_m_=std::vector<unsigned>())
		: P(P_),T_p(T_p_),T_m(T_m_)
	{
		if(T_m.size() != T_p.size())
			T_m.resize(T_p.size(),0);
	}

	virtual void Delete(){ delete this; }
	virtual ~TetraMeshBase(){}

	const std::vector<Point<3,double>>& 	points() const 	 	{ return P; }
	const std::vector<TetraByPointID>& 	tetrasByID() const 	{ return T_p; }

	virtual bool checkValid(bool printResults=false) const;

	void remapMaterial(unsigned from,unsigned to);
	std::vector<unsigned> tetraMaterialCount() const;

	unsigned getNp() const { return P.size()-1; };
	unsigned getNt() const { return T_p.size()-1; };

	// Accessors for various point/face constructs
	const Point<3,double>&  getPoint(unsigned id)           const { return P[id]; }
	const TetraByPointID&   getTetraPointIDs(unsigned id)  	const { return T_p[id]; }
    Point<3,double>         getTetraPoint(unsigned IDt,unsigned i) const { return P[T_p[IDt][i]]; }

    double                  getTetraVolume(TetraByPointID IDps) const
        { return std::abs(scalartriple(P[IDps[0]],P[IDps[1]],P[IDps[2]],P[IDps[3]])/6); }

    double                  getTetraVolume(unsigned IDt) const { return getTetraVolume(T_p[IDt]); }

    const std::vector<Point<3,double> >& getPoints() const { return P; }
    const std::vector<TetraByPointID>& getTetrasByPointID() const { return T_p; }


    unsigned                getMaterial(unsigned IDt) const { return T_m[IDt]; }
    const std::vector<unsigned>& getMaterials() const { return T_m; }

    // adjust units
    float			cmPerLengthUnit() 			const 	{ return m_cmPerLengthUnit; }
    void			cmPerLengthUnit(float l)			{ m_cmPerLengthUnit=l;		}

    array<Point<3,double>,4> tetraPoints(unsigned IDt) const
    {
    	array<Point<3,double>,4> p;
    	for(unsigned i=0;i<4;++i)
    		p[i]=P[T_p[IDt][i]];
    	return p;
    }

protected:
	std::vector<Point<3,double> >    	P;          // point vector
	std::vector<TetraByPointID>      	T_p;        // tetra -> 4 point IDs
	std::vector<unsigned>				T_m;		// tetra -> material

private:
	template<class Archive>void serialize(Archive& ar,const unsigned int version)
		{	ar & BOOST_SERIALIZATION_NVP(P) & BOOST_SERIALIZATION_NVP(T_p) & BOOST_SERIALIZATION_NVP(T_m); }

	float m_cmPerLengthUnit=1.0f;

    friend class boost::serialization::access;
};

#endif
