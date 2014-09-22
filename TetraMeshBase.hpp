#pragma once
#include <iostream>
#include <fstream>
#include <vector>

#include "newgeom.hpp"

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>

#include <boost/mpl/bool.hpp>

using namespace std;

class end_of_line_tag {
public:
};

extern end_of_line_tag end_of_line;

class ASCIITextIStream : public boost::serialization::access {
	istream& is;
public:
	typedef boost::mpl::true_ is_loading;
	typedef boost::mpl::false_ is_saving;

	void save_binary(const char *p,size_t count){}
	template<typename T>void register_type(){};
	template<typename T>void register_type(T* u){};


	ASCIITextIStream(istream& is_) : is(is_){}
	template<typename T>ASCIITextIStream& operator&(T& val){ is >> val; return *this; }
	template<typename T>ASCIITextIStream& operator&(vector<T>& v)
	{ for(T& val : v)
		is >> val;
		return *this;
	}
	template<typename T>ASCIITextIStream& operator>>(TetraByPointID& v)
	{
		for(unsigned i=0;i<4;++i)
			is >> v[i];
		return *this;
	}
	ASCIITextIStream& operator&(end_of_line_tag&){ return *this; }
};

class ASCIITextOStream : public boost::serialization::access {
	ostream& os;
public:
	typedef boost::mpl::false_ is_loading;
	typedef boost::mpl::true_  is_saving;

	template<typename T>void save_binary(const T* p,size_t count){}
	template<typename T>void register_type(){};
	template<typename T>void register_type(T* u){};
	unsigned get_library_version() const { return 0; }

	ASCIITextOStream(ostream& os_) : os(os_){}
	template<typename T>ASCIITextOStream& operator&(const T& val){ return *this << val; }

	template<typename T>ASCIITextOStream& operator<<(const T& val){ os << val; return *this; }
	template<typename T>ASCIITextOStream& operator<<(const vector<T>& v)
	{
		for (const T& val : v)
			*this & val & end_of_line;
		return *this;
	}
	template<typename T,size_t D>ASCIITextOStream& operator<<(const Point<D,T>& P)
	{
		os << P[0];
		for(unsigned i=1;i<D;++i)
			os << ' ' << P[i];
		return *this;
	}
	ASCIITextOStream& operator<<(const TetraByPointID& P)
	{
		os << P[0];
		for(unsigned i=1;i<4;++i)
			os << ' ' << P[i];

		return *this;
	}
	ASCIITextOStream& operator<<(const end_of_line_tag&){ os << endl; return *this; }
};

class TetraMeshBase {
private:
	template<class Archive>void serialize(Archive& ar,const unsigned int version)
		{	ar & BOOST_SERIALIZATION_NVP(P) & BOOST_SERIALIZATION_NVP(T_p) & BOOST_SERIALIZATION_NVP(T_m); }

	void serialize(ASCIITextOStream& ar,const unsigned int version);
	void serialize(ASCIITextIStream& ar,const unsigned int version);

protected:
	vector<Point<3,double> >    P;          // point vector
	vector<TetraByPointID>      T_p;        // tetra -> 4 point IDs
	vector<unsigned>			T_m;		// tetra -> material

public:
	const vector<Point<3,double>>& 	points() const 	 	{ return P; }
	const vector<TetraByPointID>& 	tetrasByID() const 	{ return T_p; }

	virtual bool checkValid(bool printResults=false) const;

	TetraMeshBase(){};
    TetraMeshBase(unsigned Np_,unsigned Nt_) : P(Np_+1,Point<3,double>{0,0,0}),T_p(Nt_+1,TetraByPointID{0,0,0,0}){}
	TetraMeshBase(const vector<Point<3,double> >& P_,const vector<TetraByPointID>& T_p_,const vector<unsigned>& T_m_=vector<unsigned>())
		: P(P_),T_p(T_p_),T_m(T_m_) { if(T_m.size() != T_p.size()) T_m.resize(T_p.size(),0);  }

	// Accessors for various point/face constructs
	const Point<3,double>&  getPoint(unsigned id)           const { return P[id]; }
	const TetraByPointID&   getTetraPointIDs(unsigned id)  const { return T_p[id]; }
    unsigned                getTetraFromFace(int IDf)       const;
    Point<3,double>         getTetraPoint(unsigned IDt,unsigned i) const { return P[T_p[IDt][i]]; }

    double                  getTetraVolume(TetraByPointID IDps) const
        { return abs(scalartriple(P[IDps[0]],P[IDps[1]],P[IDps[2]],P[IDps[3]])/6); }

    double                  getTetraVolume(unsigned IDt) const { return getTetraVolume(T_p[IDt]); }

    friend class boost::serialization::access;
};

TetraMeshBase loadTetraMeshBaseText(const string& fn);
void saveTetraMeshBaseText(const TetraMeshBase& M,const string& fn);
