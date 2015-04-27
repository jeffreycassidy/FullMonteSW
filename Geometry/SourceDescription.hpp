#pragma once
#include "newgeom.hpp"

#include <boost/iterator/transform_iterator.hpp>
#include <boost/random/discrete_distribution.hpp>

// Source classes (isotropic point, directed face, isotropic volume, directed point (pencil beam)
// A collection of sources can also be a Source (SourceMulti)
// Framework should be relatively simple to introduce new types

template<typename Base,typename Derived>struct cloner : public Base {
	// provide covariant clone method
	virtual Base* clone() const override { return new Derived(static_cast<Derived const&>(*this)); }

	// provide forwarding constructor
	template<typename... Args>explicit cloner(Args... args) : Base(args...){}

	// make sure destructor is virtual
	~cloner() override {}
};

class SourceDescription {
	double w;

public:

	virtual SourceDescription* clone() const=0;
	virtual ~SourceDescription(){};

	SourceDescription(double w_=1.0) : w(w_){}

	double getPower() const { return w; }
	void setPower(double w_){ w=w_; }

	virtual string operator()() const=0;
	virtual string timos_str(unsigned long long=0) const=0;
	virtual ostream& print(ostream& os) const=0;

	friend ostream& operator<<(ostream& os,const SourceDescription& sd)
	{
		sd.print(os);
		return os;
	}
};

class BallSourceDescription : public cloner<SourceDescription,BallSourceDescription> {
	Point<3,double> p0_;
	double r_=0.0;
public:

	BallSourceDescription(const Point<3,double>& p0,double r,double w_=1.0) : cloner(w_),p0_(p0),r_(r){}

	Point<3,double> getCentre() const { return p0_; }
	void setCentre(const Point<3,double> p0){ p0_=p0; }

	double getRadius() const { return r_; }
	void setRadius(double r){ r_=r; }

	virtual string operator()() const{ return "BallSourceDescription"; }
	virtual ostream& print(ostream& os) const;
	virtual string timos_str(unsigned long long=0) const { return "(invalid; ball source)"; }
};

class PointSourceDescription  {
	pair<Point<3,double>, unsigned> origin;

public:
	PointSourceDescription(const Point<3,double>& p_) : origin(make_pair(p_,0)){}

	void setOrigin(const Point<3,double> p){ origin.first=p; }
	void setOrigin(const std::array<double,3> p){ origin.first=Point<3,double>(p); }

	Point<3,double> getOrigin() const { return origin.first; }
	unsigned getTetraID() const { return origin.second; }
};

class IsotropicSourceDescription {
public:
	IsotropicSourceDescription(){}
};

class IsotropicPointSourceDescription : public IsotropicSourceDescription, public PointSourceDescription, public cloner<SourceDescription,IsotropicPointSourceDescription> {
	static constexpr double nan = std::numeric_limits<double>::quiet_NaN();
public:
	IsotropicPointSourceDescription(const Point<3,double>& p_=Point<3,double>{nan,nan,nan},double w_=1.0) : PointSourceDescription(p_), cloner(w_){};

	virtual string operator()() const { return "Isotropic point source"; }
	virtual string timos_str(unsigned long long=0) const;
	virtual ostream& print(ostream&)  const;
};

class PencilBeamSourceDescription : public cloner<SourceDescription,PencilBeamSourceDescription>, public PointSourceDescription {
	UnitVector<3,double> r;
	int IDt,IDf;
public:
	PencilBeamSourceDescription(Ray<3,double> r_,double w=1.0) : cloner(w), PointSourceDescription(r_.getOrigin()),r(r_.getDirection()){ }

	PencilBeamSourceDescription(Point<3,double> p_,UnitVector<3,double> d_,double w=1.0,int IDt_=0) : cloner(w),PointSourceDescription(p_),
			r(d_),IDt(IDt_){ }

	UnitVector<3,double> getDirection() const { return r; }

	virtual string operator()() const { return "Pencil beam source"; }

	virtual string timos_str(unsigned long long=0) const;

	virtual ostream& print(ostream&) const;

	unsigned getIDt() const { return IDt; }
};

ostream& operator<<(ostream& os,SourceDescription& src);

class VolumeSourceDescription : public IsotropicSourceDescription, public cloner<SourceDescription,VolumeSourceDescription> {
	// creates a random tetrahedral source by shearing the unit cube
protected:
	double M[3][3];
	double P0[3];
	unsigned IDt;

public:
	VolumeSourceDescription(unsigned IDt_=0,double w_=1.0) : cloner(w_),IDt(IDt_){}
	virtual string timos_str(unsigned long long=0) const;
	string operator()() const { return "Volume source"; }

	virtual ostream& print(ostream&) const;
	unsigned getIDt() const { return IDt; }
};

class FaceSourceDescription : public cloner<SourceDescription,FaceSourceDescription> {
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
	FaceSourceDescription(FaceByPointID f_,double w_=1.0) : cloner(w_),f(f_),IDt(0),IDf(0){}
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

class LineSourceDescription : public cloner<SourceDescription,LineSourceDescription> {
protected:
	Point<3,double> a,b;
public:
	LineSourceDescription(const Point<3,double>& a_,const Point<3,double>& b_,double w_=1.0) : cloner(w_),a(a_),b(b_){}

	std::pair<Point<3,double>,Point<3,double>> getEndPoints() const { return make_pair(a,b); }

	virtual string operator()() const { return "Line source"; }
	virtual string timos_str(unsigned long long=0) const { return "???Line source???"; }
	virtual ostream& print(ostream& os) const
	{
		return os << "Line source from " << a << " to " << b;
	}
};

SourceDescription* parse_string(const string& s);
