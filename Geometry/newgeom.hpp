#ifndef NEWGEOM_INCLUDED_
#define NEWGEOM_INCLUDED_
#include <array>

#include <math.h>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <utility>
#include <cassert>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <initializer_list>

using namespace std;

template<size_t D,class T>class Vector;

template<unsigned long D,class T>void rotateMax(array<T,D>& p);
template<unsigned long D,class T>void rotateMin(array<T,D>& p);

template<unsigned long D,class T>class FixedArrayID : public array<T,D> {
	public:
	FixedArrayID()                       { array<T,D>::fill(T()); }
	FixedArrayID(const T* p_)            { copy(p_,p_+D,array<T,D>::begin()); }
	FixedArrayID(const array<T,D>& a_) : array<T,D>(a_){}
	FixedArrayID(const std::initializer_list<T> il){ boost::copy(il,std::array<T,D>::begin()); }

	// find min/max elements
	unsigned findMin() const;
	unsigned findMax() const;

    unsigned getMin() const { return (*this)[findMin()]; }
    unsigned getMax() const { return (*this)[findMax()]; }

	// rotate so min/max comes first
	FixedArrayID getRotateMin() const { FixedArrayID<D,T> tmp(*this); tmp.rotateMin(); return tmp; }
	void rotateMin() { ::rotateMin((array<T,D>&)*this); }
	FixedArrayID getRotateMax() const { FixedArrayID<D,T> tmp(*this); tmp.rotateMax(); return tmp; }
	void rotateMax(){ ::rotateMax((array<T,D>&)*this); }
	FixedArrayID getSort()      const { FixedArrayID<D,T> tmp(*this); sort(tmp.begin(),tmp.end()); return tmp; }

    // orderCount counts the number of times that a[i+1] < a[i], wrapping back to include a[N-1] < a[0]
    //   for a set of 3, this indicates face ordering
    //
    // ABC / BCA / CAB all count 1
    // ACB / CBA / BAC all count 2 (opposite ordering to set above)

    unsigned orderCount() const {
    	unsigned n=0;
    	const array<T,D>& a=*this;
    	for(unsigned i=0;i<D;++i)
    		n += a[(i+1)%D] > a[i];
    	return n; }

    bool isSorted() const {
    	const array<T,D>& p=*this;
    	for(unsigned i=1;i<D;++i)
    		if(p[i-1] > p[i])
    			return false;
    	return true; }

    friend ostream& operator<<(ostream& os,const FixedArrayID& a){
    	os << '[' << a[0];
    	for(unsigned i=1;i<D;++i)
    		os << ',' << a[i];
    	return os << ']';
    }
};

class FaceByPointID;

class TetraByPointID : public FixedArrayID<4,unsigned> {
	public:
	TetraByPointID()                	: FixedArrayID<4,unsigned>()  { }
	TetraByPointID(const unsigned* p_)  : FixedArrayID<4,unsigned>(p_){ }
    TetraByPointID(const array<unsigned,4>& p_) : FixedArrayID<4,unsigned>(p_) { }
    TetraByPointID(unsigned p0,unsigned p1,unsigned p2,unsigned p3){ (*this)[0]=p0; (*this)[1]=p1; (*this)[2]=p2; (*this)[3]=p3; }

	FaceByPointID getFace         (unsigned faceNum);
	//unsigned      getOppositePoint(unsigned faceNum) const;
};

class TetraByFaceID : public FixedArrayID<4,int> {
	public:
	TetraByFaceID()              : FixedArrayID<4,int>(){}
	TetraByFaceID(const int* p_) : FixedArrayID<4,int>(p_){}
	TetraByFaceID(std::initializer_list<int> il) : FixedArrayID<4,int>(il){}

	void flipFace(unsigned faceNum){
		array<int,4>& p=*this;
		p[faceNum]=-p[faceNum];
	}

	pair<bool,unsigned> getFace(unsigned faceNum) const {
		const array<int,4>&p = *this;
		bool inv=p[faceNum]<0;
		unsigned id=inv?-p[faceNum] : p[faceNum];
		return make_pair(inv,id); }
};

class FaceByPointID : public FixedArrayID<3,unsigned> {
	public:
	FaceByPointID()                   { fill(0U); }
	FaceByPointID(const unsigned* p_) { copy(p_,p_+3,begin()); }
	FaceByPointID(const FixedArrayID<3,unsigned>& t_) : FixedArrayID<3,unsigned>(t_) {}
	FaceByPointID(unsigned a,unsigned b,unsigned c){ array<unsigned,3>& p=*this; p[0]=a; p[1]=b; p[2]=c; }

    // flips the orientation of the face [A,B,C] -> [A,C,B]
    FaceByPointID flip() const { const array<unsigned,3>&p =*this; return FaceByPointID(p[0],p[2],p[1]); }
};

template<size_t D,class T>unsigned findMin(const array<T,D>& p)
{
	unsigned m=std::numeric_limits<T>::max(),j=0;
	for(unsigned i=0;i<D;++i)
	{
		if (p[i]<m){
			m=p[i];
			j=i;
		}
	}
	return j;
}

template<size_t D,class T>unsigned findMax(const array<T,D>& p)
{
	unsigned m=std::numeric_limits<T>::min(),j=0;
	for(unsigned i=0;i<D;++i)
	{
		if (p[i]>m){
			m=p[i];
			j=i;
		}
	}
	return j;
}

template<size_t D,class T>void rotateMin(array<T,D>& p)
{
	array<T,D> tmp(p);
	unsigned j=findMin(p);
	for(unsigned i=0;i<D;++i)
		p[i]=tmp[(i+j)%D];
}

template<size_t D,class T>void rotateMax(array<T,D>& p)
{
	array<T,D> tmp(p);
	unsigned j=findMax(p);
	for(unsigned i=0;i<D;++i)
		p[i]=tmp[(i+j)%D];
}


template<size_t D,class T>class Point : public array<T,D>
{
	public:

	using std::array<T,D>::operator=;

	Point()                { array<T,D>::fill(T()); };
	Point(const std::initializer_list<double>& il_){ copy(il_.begin(),il_.end(),array<T,D>::begin()); }
	Point(const Point& P_) : array<T,D>(P_){};
	Point(const T* p_)     { copy(p_,p_+3,array<T,D>::begin()); }

	template<typename U>explicit Point(const std::array<U,D> a){ boost::copy(a,array<T,D>::begin()); }
	template<typename U>explicit Point(const U* p){ boost::copy(p,array<T,D>::begin()); }

	Point operator+(const Vector<D,T>& v) const { Point t; for(unsigned i=0;i<D;++i){ t[i]=(*this)[i]+v[i]; } return t; }
	Point operator-(const Vector<D,T>& v) const { Point t; for(unsigned i=0;i<D;++i){ t[i]=(*this)[i]-v[i]; } return t; }

	friend ostream& operator<<(ostream& os,const Point<D,T>& P)
	{
		os << P[0];
		for(unsigned i=1;i<D;++i)
			os << ',' << P[i];
		return os;
	}
};

template<size_t D,class T>std::ostream& operator<<(std::ostream& os,const array<T,D>& P)
{
	os << '(' << P[0];
	for(int i=0; i<D; ++i){ os << ',' << P[i]; }
	return os;
}


template<size_t D,class T>std::istream& operator>>(std::istream& is,Point<D,T>& P)
{
	is >> std::skipws;
	bool paren=false;

	if(is.peek()=='('){ paren=true; is.ignore(1); }
	for(size_t i=0; i<D; ++i){ is >> P[i]; if(i < D-1 && is.peek()==','){ is.ignore(1); } }
	if (paren){ is.ignore(1); }
	return is;
}


// A Vector extends the Point class with a norm, dot product, cross product, add/sub and scalar multiply/divide
//    vector can be defined as going between two points, or implicitly as the origin (0,0,0) to a point
template<size_t D,class T>class Vector : public Point<D,T>
{
	public:
	using Point<D,T>::operator=;
	using Point<D,T>::operator[];
	using Point<D,T>::operator-;
	using Point<D,T>::operator+;

	Vector()                                        : Point<D,T>()  {};
	Vector(const T* x_)                             : Point<D,T>(x_){};
	Vector(const Point<D,T>& P)                     : Point<D,T>(P) {};
	Vector(const Point<D,T>& A,const Point<D,T>& B){
		for(size_t i=0;i<D;++i)
			(*this)[i]=B[i]-A[i];
	}

	// norms and dots
	T norm_l2()                 const { T s=0; for(size_t i=0;i<D;++i){ s += (*this)[i] * (*this)[i]; } return(sqrt(s)); }
	T norm_l1()                 const { T s=0; for(size_t i=0;i<D;++i){ s += abs((*this)[i]); } return abs(s);   }
	T norm2_l2()                const { T s=0; for(size_t i=0;i<D;++i){ s += (*this)[i]*(*this)[i]; } return s;        }
	T dot(const Vector<D,T>& a) const { T s=0; for(size_t i=0;i<D;++i){ s += a[i]*(*this)[i]; } return s;        }

	// unary negate
	Vector operator-()           { Vector v; for(size_t i=0; i<D; ++i){ v[i]=-(*this)[i]; } return v; }

	// vector += / -= operations
	const Vector& operator+=(const Vector& k) { for(int i=0; i<D; ++i){ (*this)[i]+=k[i]; } return *this; }
	const Vector& operator-=(const Vector& k) { for(int i=0; i<D; ++i){ (*this)[i]-=k[i]; } return *this; }

	// scalar operations
	const Vector& operator*=(const T& k) { for(size_t i=0; i<D; ++i){ (*this)[i]*=k; } return *this; }
	const Vector& operator/=(const T& k) { for(size_t i=0; i<D; ++i){ (*this)[i]/=k; } return *this; }
	Vector operator* (T k)        const { Vector<D,T> t(*this); return t *= k; }
	Vector operator/ (const T& k) const { Vector<D,T> t(*this); return t /= k; }

	// vector cross product
	Vector cross(const Vector<D,T>&) const;
	template<size_t D_,class U>friend Vector<D_,U> cross(const Vector<D_,U>&,const Vector<D_,U>&);
};

// helpers
template<size_t D,class T>T dot(const Vector<D,T>& a,const Vector<D,T>& b) { return a.dot(b); }
template<size_t D,class T>T norm_l2(const Vector<D,T>& a) { return a.norm_l2(); }
template<size_t D,class T>T norm_l1(const Vector<D,T>& a) { return a.norm_l1(); }
template<size_t D,class T>T norm2_l2(const Vector<D,T>& a){ return a.norm2_l2(); }

template<size_t D,class T>T norm2_l2(const Point<D,T>& a,const Point<D,T>& b){ T s; for(unsigned i=0;i<D;++i){ s += (a[i]-b[i])*(a[i]-b[i]); } return s; }


// A UnitVector is a Vector that is guaranteed to always have L2 norm 1
template<size_t D,class T>class UnitVector : public Vector<D,T>
{
	public:
	using Vector<D,T>::dot;
	using Vector<D,T>::norm_l2;
	using Vector<D,T>::norm2_l2;
    using Vector<D,T>::operator-;
	using Vector<D,T>::operator*;
	using Vector<D,T>::operator/;

	UnitVector()                    { (*this)[0]=1;          for(unsigned i=1;i<D;++i){ (*this)[i]=0;      } };
    UnitVector(const T* v_,bool alreadyUnit=false){
        for (unsigned i=0;i<D;++i)
        	(*this)[i]=v_[i];
        if (!alreadyUnit)
        {
            T L = norm_l2();
            for (unsigned i=0;i<D;++i)
            	(*this)[i] /= L;
        }
    }
	UnitVector(const Vector<D,T>& v,bool alreadyUnit=false){
        if (alreadyUnit)
            for(unsigned i=0;i<D;++i)
            	(*this)[i]=v[i];
        else
        {
            T L=v.norm_l2();
            for(unsigned i=0;i<D;++i)
            	(*this)[i]=v[i]/L;
        }
        //assert(norm_l2()==eps);
    };
    UnitVector(const UnitVector<D,T>& v){
        for(unsigned i=0;i<D;++i)
        	(*this)[i]=v[i];
    };
    UnitVector(const std::initializer_list<T>& il){ copy(il.begin(),il.end(),Vector<D,T>::begin()); }

    UnitVector operator-() const { UnitVector t(*this); for(unsigned i=0;i<D;++i) t[i] = -t[i]; return t; }

    friend ostream& operator<<(ostream& os,const UnitVector& uv)
    {
    	os << '<' << uv[0];
    	for(unsigned i=1;i<D;++i)
    		os << ',' << uv[i];
    	return os << '>';
    }
};

UnitVector<3,double> uvect3FromPolar(double phi,double lambda);

template<size_t D,class T> Vector<D,T> cross(const Vector<D,T>& a,const Vector<D,T>& b)
{
	return a.cross(b);
}

template<size_t D,class T>Vector<D,T> Vector<D,T>::cross(const Vector<D,T>& x) const
{
	T cp[3]= { (*this)[1]*x[2]-(*this)[2]*x[1], (*this)[2]*x[0]-x[2]*(*this)[0], (*this)[0]*x[1]-(*this)[1]*x[0] };
	return Vector<D,T>(cp);
}

// Computes the scalar triple product for points A,B,C,D = DA dot (DB cross DC)
template<class T>T scalartriple(const Point<3,T>& A,const Point<3,T>& B,const Point<3,T>& C,const Point<3,T>& D)
{
    return scalartriple(Vector<3,T>(D,A),Vector<3,T>(D,B),Vector<3,T>(D,C));
}

// Computes the scalar triple product for vectors a,b,c = a dot (b cross c)
template<class T>T scalartriple(const Vector<3,T>& a,const Vector<3,T>& b,const Vector<3,T>& c)
{
    return dot(a,cross(b,c));
}


// A Ray represents the geometric idea of a ray, a semi-infinite line starting from a point and extending along a unit vector
template<size_t D,class T>class Ray
{
	Point<D,T>      P;
	UnitVector<D,T> d;

	public:
	// construct from an origin point and a direction unit vector
    Ray(){};
	Ray(const Point<D,T>& P_,const UnitVector<D,T>& d_) : P(P_),d(d_){};

	// returns the point that is T units along the ray
	Point<D,T> operator()(T t) const { return Point<D,T>(P+d*t); };

	const Point<D,T>&      getOrigin()    const { return P; }
	const UnitVector<D,T>& getDirection() const { return d; }

    void setOrigin(Point<3,double> p_){ P=p_; }

	void print(ostream& os) const { os << "Ray: " << P << " " << d << std::endl; }
};

//Point<3,double> pointFrom(__m128 p);
//Ray<3,double> rayFrom(__m128 p,__m128 d);


//UnitVector<3,double> uvectFrom(__m128 v);

// returns true if P+td falls within triangle defined by points T for some t
bool PointInTriangle(Point<3,double> p,UnitVector<3,double> d,Point<3,double> T[3],Point<3,double>& Q,double& t);

// IO manipulator for printing points (parentheses or not, commas or not, etc)
class GeomManip {
    char delimchar;
    bool parens;
    char parenchar[2];
    char uvparenchar[2];
    char idparenchar[2];
    ostream& os;
    public:

    explicit GeomManip(ostream& os_=cout) : delimchar(','),parens(false),os(os_){
        parenchar[0]='(';     parenchar[1]=')';
        uvparenchar[0] = '<'; uvparenchar[1]='>';
        idparenchar[0] = '['; idparenchar[1]=']';}

    static GeomManip plainwhite(){
        GeomManip gm;
        gm.parens=false;
        gm.delimchar=' ';
        return gm;
    }

    friend GeomManip operator<<(ostream&,const GeomManip&);

    template<size_t D,class T>friend ostream& operator<<(const GeomManip& gm,const Point<D,T>& p);
};

template<size_t D,class T>ostream& operator<<(const GeomManip& gm,const UnitVector<D,T>& u)
{
    if(gm.parens)
        gm.os << gm.uvparenchar[0];
    for(unsigned i=0;i<(unsigned)D; ++i)
    {
        gm.os << u[i];
        if (i<D-1)
            gm.os << gm.delimchar;
    }
    if(gm.parens)
        gm.os << gm.uvparenchar[1];
    return gm.os;
}

template<size_t D,class T>ostream& operator<<(const GeomManip& gm,const Point<D,T>& p)
{
    if(gm.parens)
        gm.os << gm.parenchar[0];
    for(unsigned i=0;i<(unsigned)D; ++i)
    {
        gm.os << p[i];
        if (i<D-1)
            gm.os << gm.delimchar;
    }
    if(gm.parens)
        gm.os << gm.parenchar[1];
    return gm.os;
}

extern GeomManip plainwhite;

std::istream& operator>>(std::istream& is,TetraByPointID& P);
std::istream& operator>>(std::istream& is,FaceByPointID& F);
std::ostream& operator<<(std::ostream& os,TetraByPointID& T);
std::ostream& operator<<(std::ostream& os,FaceByPointID& T);

#endif
