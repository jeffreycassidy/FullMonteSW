#pragma once
#include "newgeom.hpp"

#include <boost/iterator/transform_iterator.hpp>
#include <boost/random/discrete_distribution.hpp>

// Source classes (isotropic point, directed face, isotropic volume, directed point (pencil beam)
// A collection of sources can also be a Source (SourceMulti)
// Framework should be relatively simple to introduce new types

class SourceDescription {
	double w;

public:
	SourceDescription(double w_=1.0) : w(w_){}
	double getPower() const { return w; }

	virtual string operator()() const=0;
	virtual string timos_str(unsigned long long=0) const=0;
	virtual ostream& print(ostream& os) const=0;

	friend ostream& operator<<(ostream& os,const SourceDescription& sd)
	{
		sd.print(os);
		return os;
	}
};

class PointSourceDescription : virtual public SourceDescription {
	pair<Point<3,double>, unsigned> origin;

public:
	PointSourceDescription(const Point<3,double>& p_,double w_=1.0) : SourceDescription(w_),origin(make_pair(p_,0)){}

	Point<3,double> getOrigin() const { return origin.first; }
	unsigned getTetraID() const { return origin.second; }
};

class IsotropicSourceDescription : virtual public SourceDescription {
public:
	IsotropicSourceDescription(double w_=1.0) : SourceDescription(w_){}
};

class IsotropicPointSourceDescription : public IsotropicSourceDescription, public PointSourceDescription {
public:
	IsotropicPointSourceDescription(Point<3,double>& p_,double w_=1.0) : PointSourceDescription(p_,w_){};

	virtual string operator()() const { return "Isotropic point source"; }
	virtual string timos_str(unsigned long long=0) const;
	virtual ostream& print(ostream&)  const;
};

class PencilBeamSourceDescription : public PointSourceDescription {
	UnitVector<3,double> r;
	int IDt,IDf;
public:

	PencilBeamSourceDescription(Ray<3,double> r_,double w=1.0) : PointSourceDescription(r_.getOrigin(),w),r(r_.getDirection()){ }

	PencilBeamSourceDescription(Point<3,double> p_,UnitVector<3,double> d_,double w=1.0,int IDt_=0) : PointSourceDescription(p_,w),
			r(d_),IDt(IDt_){ }

	UnitVector<3,double> getDirection() const { return r; }

	virtual string operator()() const { return "Pencil beam source"; }

	virtual string timos_str(unsigned long long=0) const;

	virtual ostream& print(ostream&) const;

	unsigned getIDt() const { return IDt; }
};

ostream& operator<<(ostream& os,SourceDescription& src);

class VolumeSourceDescription : public IsotropicSourceDescription {
	// creates a random tetrahedral source by shearing the unit cube
protected:
	double M[3][3];
	double P0[3];
	unsigned IDt;

public:
	VolumeSourceDescription(unsigned IDt_=0,double w=1.0) : SourceDescription(w),IDt(IDt_){}
	virtual string timos_str(unsigned long long=0) const;
	string operator()() const { return "Volume source"; }



	virtual ostream& print(ostream&) const;
	unsigned getIDt() const { return IDt; }
};

class FaceSourceDescription : virtual public SourceDescription {
protected:
	FaceByPointID f;
	unsigned IDt;
	int IDf;
	UnitVector<3,double> n;
	Point<3,double> P0;
	double M[3][2];

public:

	// if force_boundary is set, requires the face to be pointing in from an object boundary
	//    virtual bool prepare(const TetraMesh& m) { return prepare(m,true); }
	//    bool prepare(const TetraMesh&,bool force_boundary=true);

	double w;
public:
	FaceSourceDescription(FaceByPointID f_,double w_=1.0) : SourceDescription(w_),f(f_),IDt(0),IDf(0){}
	virtual string operator()() const { return "Face Source"; }

	virtual ostream& print(ostream& os) const;
	virtual string timos_str(unsigned long long=0) const;

	unsigned getIDt() const { return IDt; }
	FaceByPointID getIDps() const { return f; }
};

class SourceMultiDescription : virtual public SourceDescription {
	vector<SourceDescription*> sources;
	double w_total;

public:
	// Create from a pair of iterators that dereference to a Source*
	SourceMultiDescription(){}
	template<class ConstIterator> SourceMultiDescription(ConstIterator begin,ConstIterator end) :
	sources(begin,end),
			w_total(0.0)
			{ for(; begin != end; ++begin) w_total += (*begin)->getPower(); }

	virtual string operator()() const { return "Multiple sources"; }
	virtual string timos_str(unsigned long long=0) const;

	virtual ostream& print(ostream& os) const;
};
