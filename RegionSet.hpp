#include <vector>
#include <iostream>
#include <functional>
#include <algorithm>

class RegionSet {
	std::vector<unsigned> R;			///< Map of element to region
	unsigned Nr;						///< Number of regions
public:
	static void Assigner(RegionSet* rs_,unsigned r_,unsigned el_){ rs_->R[el_]=r_; }
	//static bool Membership(RegionSet* rs_,unsigned r_,unsigned el_){ return rs_->R[el_]==r_; }

public:
	RegionSet() : R(),Nr(0){}
	RegionSet(unsigned Ne_,unsigned Nr_=0) : R(Ne_,0),Nr(Nr_){}
	RegionSet(const RegionSet&) =delete;
	RegionSet(const RegionSet&&)=delete;
	RegionSet(const std::vector<unsigned>& v_,unsigned Nr_=0) : R(v_),Nr(Nr_ ? Nr_ : *std::max_element(v_.begin(),v_.end())){}

	///
	// NOTE: Need to pass this as pointer; when do it by reference it invokes some kind of funny copy semantics; not what you want!
	auto AssignElementsToRegion(unsigned r_) -> decltype(std::bind(&Assigner,this,r_,std::placeholders::_1))
			{ return std::bind(&Assigner,this,r_,std::placeholders::_1); }

	//auto IsElementMemberOfRegion(unsigned r_) -> decltype(std::bind(&Membership,this,r_,std::placeholders::_1))
//		{ return std::bind(&Membership,this,r_,std::placeholders::_1); }

	typedef std::vector<unsigned>::const_iterator const_iterator;

	const_iterator begin() const { return R.begin(); }
	const_iterator end()   const { return R.end();   }

	std::vector<unsigned> getRegionElementCounts() const;

	void print(std::ostream& os) const;
};
