#pragma once
#include "SourceDescription.hpp"
#include "SourceEmitter.hpp"
#include "graph.hpp"
#include <math.h>

#include <boost/math/constants/constants.hpp>

inline __m128 to_m128(const UnitVector<3,double>& uv)
{
	return _mm_set_ps(0.0,uv[2],uv[1],uv[0]);
}

template<class RNG>class LineSourceEmitter : virtual public SourceEmitter<RNG>,virtual public LineSourceDescription {
	vector<pair<double,unsigned>> regions;	///< Vector giving the ends of each region segment along the source in normalized coords 0..1

	Vector<3,double> v;					///< Vector from one side to the other
	UnitVector<3,double> uv;			///< Unit vector along the source axis

	UnitVector<3,double> n0,n1;			///< Two mutually orthogonal unit vectors normal to the source axis

public:

	LineSourceEmitter(const TetraMesh& mesh_,const LineSourceDescription& lsd) :
		LineSourceEmitter(mesh_,lsd.getEndPoints().first,lsd.getEndPoints().second){};
	LineSourceEmitter(const TetraMesh& mesh_,const Point<3,double>& a_,const Point<3,double>& b_);


	/// Calculate the origin point and region ID for a given random value in [0,1)
	inline std::pair<Point<3,double>,unsigned> getOrigin(double t_) const;

	/// Draw a [0,1) random number and evaluate the starting point
	std::pair<Point<3,double>,unsigned> getOrigin(RNG& rng) const { double t=rng.draw_float_u01(); return getOrigin(t); }

	/// Draw a U[0,1) random variable and calculate the resulting direction
	array<UnitVector<3,double>,3> getDirection(double r) const {
		double twopitheta=2*boost::math::constants::pi<double>()*r;
		return getDirection(sin(twopitheta),cos(twopitheta));
	}

	/// Calculate the direction given a sine and cosine forming a unit vector
	array<UnitVector<3,double>,3> getDirection(double sintheta,double costheta) const;

	// Comparison function used for comparing a normalize distance b [0,1) along the segment with a region marker a
	static bool CompareDistance(const pair<double,unsigned>& a,double b){ return a.first < b; }

	// Standard emission routine
	pair<Packet,unsigned> emit(RNG& rng) const;
};



template<class RNG>LineSourceEmitter<RNG>::LineSourceEmitter(const TetraMesh& mesh_,const Point<3,double>& a_,const Point<3,double>& b_)
				: SourceEmitter<RNG>(mesh_),LineSourceDescription(a_,b_),
				  v(a_,b_),
				  uv(v)
{
	// define emission normals
	n0 = UnitVector<3,double>(cross(uv,UnitVector<3,double>{1.0,0.0,0.0}));
	n1 = cross(n0,uv);

	cout << "Unit vectors are: " << n0 << " and " << n1 << endl;
	cout << "  Dot products: " << dot(n0,n1) << ", " << dot(n0,uv) << ", " << dot(n1,uv) << endl;

	// walk the line and find regions
	unsigned IDt=mesh_.findEnclosingTetra(a_);
	Tetra T=mesh_.getTetra(IDt);
	double L=norm_l2(v);
//
//	cout << "Calculating line source from " << a_ << " to " << b_ << endl;
//	cout << "  Length: " << L << endl;
//	cout << "  Vector: " << v << endl;
//	cout << "  Direction: " << uv << endl;
//	cout << "  Starting at distance 0, tetra " << IDt << endl;

	__m128 p=_mm_set_ps(0.0,a_[2],a_[1],a_[0]);
	__m128 d=_mm_set_ps(0.0,uv[2],uv[1],uv[0]);
	float l=0.0,dist;		// distance travelled

	for(unsigned N=0; l<L-1e-6 && IDt != 0; ++N)
	{
		__m128 s=_mm_set1_ps(L-l);
		StepResult res = T.getIntersection(p,d,s);

		_mm_store_ss(&dist,res.distance);
		l += dist;

		cout << "Entered tetra " << IDt << " at distance " << L-l << endl;
		regions.emplace_back(l/L,IDt);

		IDt=res.IDte;
		p=res.Pe;
		T=mesh_.getTetra(IDt);
	}

	if (IDt == 0)
		cerr << "ERROR: Line source is not fully contained within geometry" << endl;
}

template<class RNG>pair<Packet,unsigned> LineSourceEmitter<RNG>::emit(RNG& rng) const
{
	unsigned IDt;
	Point<3,double> p0;
	Packet pkt;

	float f[4];

	std::tie(p0,IDt) = getOrigin(rng.draw_float_u01());

	_mm_store_ps(f,rng.draw_m128f2_uvect());
	pkt.setPosition(p0);
	pkt.setDirection(getDirection(f[0],f[1]));

	return make_pair(pkt,IDt);
}

template<class RNG>pair<Point<3,double>,unsigned> LineSourceEmitter<RNG>::getOrigin(double t_) const
{
	vector<pair<double,unsigned>>::const_iterator it=lower_bound(regions.begin(),regions.end(),t_,CompareDistance);
	if (it == regions.end())
		cerr << "ERROR: Went off the end of the range" << endl;
	unsigned IDt=it->second;
	Point<3,double> p=a+v*t_;
	return make_pair(p,IDt);
}

template<class RNG>array<UnitVector<3,double>,3> LineSourceEmitter<RNG>::getDirection(double sintheta_,double costheta_) const
{
	return array<UnitVector<3,double>,3>{
		UnitVector<3,double>(n0*sintheta_+n1*costheta_,true),
		UnitVector<3,double>(n0*costheta_-n1*sintheta_,true),
		uv
	};
}
