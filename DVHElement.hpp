#include <cmath>
#include <cstddef>
#include <iostream>

class DVHElement {
	unsigned IDt=0;
	unsigned region=0;
	double fluence=NAN;
	double fluence_var=NAN;
	double volume=NAN;
	double volume_sum=NAN;
	double volume_cdf=NAN;

public:

	DVHElement(){}

	explicit DVHElement(unsigned int IDr_) : region(IDr_){}

	DVHElement(unsigned IDt_,unsigned region_,double volume_=NAN,double fluence_=NAN,double fluence_var_=NAN) :
		IDt(IDt_),
		region(region_),
		fluence(fluence_),
		fluence_var(fluence_var_),
		volume(volume_){}

	DVHElement(const DVHElement&)=default;
	DVHElement& operator=(const DVHElement&) = default;

	double getVolume() const { return volume; }
	double getVolumeCDF() const { return volume_cdf; }
	double getFluence() const { return fluence; }
	double getFluenceVar() const { return fluence_var; }
	double getFluenceStd() const { return sqrtf(fluence_var); }


	/**
	 * NOTE: Volume-weighted fluence calculation is correct _only_ when material absorption coeff is uniform.
	 * Relies on the constancy of mu_a in Ex[phi] = Ex[E]/mu_a/V
	 */

	class DVHAccumulator {
		double sum_v=0;
		double sum_phi_V=0;
		double *o,*o_phi;

	public:
		DVHAccumulator(double* o_=NULL,double* o_phi_=NULL) : sum_v(0),sum_phi_V(0),o(o_),o_phi(o_phi_){}
		~DVHAccumulator(){
			if (o) *o=sum_v;
			if (o_phi) *o_phi = sum_v == 0 ? 0.0 : sum_phi_V/sum_v;
		}

		typedef DVHElement type;
		typedef DVHElement result_type;

		DVHElement operator()(const DVHElement& d0){
			DVHElement d(d0);
			d.volume_sum = (sum_v += d0.volume);
			sum_phi_V += d0.fluence;
			return d;  }
	};

	class NormalizeVolume {
		double k;
	public:
		NormalizeVolume(double v_) : k(1.0/v_){}

		DVHElement operator()(const DVHElement& d0) const { DVHElement d(d0); d.volume_cdf = 1.0-d0.volume_sum*k; return d; }
	};

	// static comparison methods
	static bool DVHOrder(const DVHElement& a,const DVHElement& b)
		{ return a.region < b.region || (a.region==b.region && a.fluence<b.fluence); }

	static bool FluenceOrder(const DVHElement& a,const DVHElement& b)
		{ return a.fluence<b.fluence; }

	static bool RegionOrder(const DVHElement& a,const DVHElement& b)
		{ return a.region < b.region; }

	// note: sense of comparison is backwards because volume fraction is descending
	static bool VolumeCDFCompare(const DVHElement& a,double x_){ return a.volume_cdf > x_; }

	friend class DVH;
};



