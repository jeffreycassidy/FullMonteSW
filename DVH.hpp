#include <vector>
#include "DVHElement.hpp"
#include "graph.hpp"
#include <iomanip>
#include <functional>

#include <boost/random/normal_distribution.hpp>

using namespace std;

template<class Container>class SubRangeAdaptor;
template<class Container>ostream& operator<<(ostream& os,const SubRangeAdaptor<Container>& a);


/** Adaptor which allows access to a subrange of a random access container. Uses a reference to the base container to avoid
 * iterator invalidation in the event of insertion/deletion/resizing.
 *
 */

template<class Container>class SubRangeAdaptor {
	unsigned idx_start,idx_end;
	Container& c;

public:
	SubRangeAdaptor(Container& c_,unsigned idx_start_,unsigned idx_end_) : idx_start(idx_start_),idx_end(idx_end_),c(c_){}
	typedef typename Container::iterator iterator;
	typedef typename Container::const_iterator const_iterator;
	typedef typename Container::reference reference;
	typedef typename Container::const_reference const_reference;

	unsigned getStartIdx() const { return idx_start; }
	unsigned getEndIdx()   const { return idx_end;   }

	// Iterator access
	iterator begin(){ return c.begin()+idx_start; }
	iterator   end(){ return c.begin()+idx_end; }
	const_iterator begin() const { return c.begin()+idx_start; }
	const_iterator   end() const { return c.begin()+idx_end;   }
	const_iterator cbegin() const { return c.cbegin()+idx_start; }
	const_iterator   cend() const { return c.cbegin()+idx_end;   }

	// Subrange size
	unsigned size() const { return idx_end-idx_start; }

	// Front/back accessors
	reference		front()			{ return c[idx_start]; }
	const_reference front() const 	{ return c[idx_start]; }
	reference 		back()			{ return c[idx_end-1]; }
	const_reference back() const 	{ return c[idx_end-1]; }

	// Element accessors
	reference&  operator[](unsigned i) { return c[i+idx_start]; }
	const_reference& operator[](unsigned i) const { return c[i+idx_start]; }

	friend ostream& operator<<<>(ostream&,const SubRangeAdaptor&);
};

template<class Container>SubRangeAdaptor<Container> make_subrange(Container& c,unsigned idx_start,unsigned idx_end)
		{ return SubRangeAdaptor<Container>(c,idx_start,idx_end); }



/** Class to hold the dose-volume histogram.
 * Formally, the dose-volume histogram shows percentage of volume (y-axis %V) against dose (x-axis D), where the volume
 * is the percentage receiving a dose >= D.
 *
 * Though not explicitly done, the point (0,1) should always be included.
 * Likewise, the last point should step down to zero.
 *
 * Octave (and Matlab?) extend rightward from specified points
 *
 * TODO: Would be nice to compress out leading (0, y) if there are no-dose regions
 */

class DVH {
	class DVHRegion : public SubRangeAdaptor<vector<DVHElement>> {
		double V;
		double phi_bar;

	public:
		DVHRegion(vector<DVHElement>& dvh_,unsigned idx_start_=0,unsigned idx_end_=0,double V_=NAN,double phi_bar_=NAN) :
			SubRangeAdaptor<vector<DVHElement>>(dvh_,idx_start_,idx_end_),
			V(V_),
			phi_bar(phi_bar_){}

		double getVolume() const { return V; }
		double getMeanFluence() const { return phi_bar; }

		double operator()(double vol_cdf_) const {
			const_iterator it = lower_bound(cbegin(),cend(),vol_cdf_,DVHElement::VolumeCDFCompare);

			cout << "Volume CDF value " << vol_cdf_ << " is bracketed by ";

			if (it == cbegin())
				cout << setprecision(6) << it->volume_cdf << ",1.0)";
			else if (it == cend())
				cout << "[0," << setprecision(6) << it->volume_cdf << ')';
			else
				cout << '[' << setprecision(6) << it->volume_cdf << ',' << setprecision(6) << (it-1)->volume_cdf << ')' << endl;
			cout << endl;

			return 0;
		}

		friend class DVH;
	};

	vector<DVHElement> dvh;
	vector<DVHRegion> regions;

	void sort();				///< Sorts the DVH for the first time (no assumptions on input data)
	void calcVolumeCDF();		///< Does the running sum of volume elements to get volume CDF

	DVHRegion& region(unsigned i){ return regions[i]; }

public:

	/// Move constructor from vector<DVHElement>; may specify whether already sorted or not, though currently ignored
	DVH(vector<DVHElement>&& dvh_,bool sorted_=false) : dvh(std::move(dvh_))
		{ if(!sorted_)
			sort();
		}

	/// Copy constructor from an existing vector of DVHElements
	DVH(const vector<DVHElement>& dvh_) : dvh(dvh_){ sort(); }

	/// Construct from an existing TetraMesh, fluence vector, and region assignment
	DVH(const TetraMesh& M,const vector<double>& fluence,const vector<unsigned>& regions_);

	/// Trivial copy constructor
	DVH(const DVH& dvh_) : dvh(dvh_.dvh){
		regions.reserve(dvh_.regions.size());
		for(const DVHRegion& R : dvh_.regions)
			regions.emplace_back(dvh,R.getStartIdx(),R.getEndIdx());
	}

	const DVHRegion& cregion(unsigned i) const { return regions[i]; }

	void writeASCII(string fn,bool incl_var_=false) const;

	void DVHCheck() const;

	template<class RNG>DVH bootstrap(RNG& rng) const;

	friend class DVHRegion;
};



/** Bootstraps a new DVH from the current one using normally-distributed zero-mean noise and the fluence_var variance values
 * present in the DVH.
 *
 * @tparam RNG		A random number generator which implements operator() to generate uniform random numbers,
 * 	for instance boost::mt19937
 */

template<class RNG>DVH DVH::bootstrap(RNG& rng) const
{
	DVH dvh_out(*this);

	boost::random::normal_distribution<double> norm_rng;

	for(DVHElement& el : dvh_out.dvh)
		el.fluence += norm_rng(rng,boost::normal_distribution<double>::param_type(0,sqrt(el.fluence_var)));

	dvh_out.sort();

	return dvh_out;
}

template<class Container>ostream& operator<<(ostream& os,const SubRangeAdaptor<Container>& a)
{
	return os << '[' << a.idx_start << ',' << a.idx_end << ')';
}
