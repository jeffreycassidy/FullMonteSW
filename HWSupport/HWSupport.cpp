/*
 * HWSupport.cpp
 *
 *  Created on: Dec 18, 2014
 *      Author: jcassidy
 */

//#include <boost/multiprecision/gmp.hpp>
#include "../fm-postgres/fmdbexportcase.hpp"
#include <iostream>

using namespace std;

unsigned long long to_fixpoint(double d,bool signed_,int i,int f)
{
	bool neg = d < 0;

	if (f<0)
	{
		f=-f;
		d /= double(1ULL << f);
	}
	else
		d *= double(1ULL << f);

	unsigned long long mask = (1ULL << (i+f)) - 1ULL;

	return llrint(d) & mask;
}

struct HWMaterial {
	uint32_t mu_t;
	uint32_t recip_mu_t;
	uint32_t absfrac;
	uint32_t prop;
	uint64_t hg_consts;
};

HWMaterial convert_to_hw(const Material& mat)
{
	static constexpr double ln2       = log(2.0);
	static constexpr double recip_ln2 = 1.0/ln2;

	HWMaterial hwmat;

	double g = mat.getg();

	hwmat.mu_t 			= to_fixpoint(mat.getMuT()*recip_ln2,false,12,6);
	hwmat.recip_mu_t 	= to_fixpoint(1.0/mat.getMuT()*ln2,false,6,12);
	hwmat.absfrac		= to_fixpoint(mat.getAbsorbedFraction(),false,0,18);
	hwmat.hg_consts		= (to_fixpoint((1.0+g*g)/2.0/g,false,-2,20) << 36) |
							(to_fixpoint((1.0-g*g)/sqrtf(2.0*g),false,0,18) << 18) |
							to_fixpoint(g,false,0,18);

	hwmat.prop=0;
	return hwmat;
}

boost::multiprecision::number<gmp_int>
{
	boost::multiprecision::number<gmp_int> t;

	// 4 TetraAdjacency, each:		(112)
	//		InterfaceID		(u8)
	//		TetraID			(u20)
	// 4 FaceDef, each:				(296)
	//		c (s4.14)
	//		n 3x (s0.18)
	// 1 MaterialID					  (4)
	//
	//	TOTAL						(412)

	for(unsigned i=0;i<;++i)
		t |= ;

	return t;
}

int main(int argc,char **argv)
{
	boost::shared_ptr<PGConnection> conn = PGConnect();

	vector<Material> mats;

	exportMaterials(*conn,2,mats);

	vector<HWMaterial> hwmats(mats.size());

	cout << hex;

	// NEED TO FOLD IN 1/log(2) to mu_t's because RNG gives log2(X) not ln(X)
	for(unsigned i=0;i<mats.size(); ++i)
	{
//		double g = mats[i].getg();
//		unsigned long long hg_const0, hg_const1, hg_g;
//		cout << "Material mu_t=" << to_fixpoint(mats[i].getMuT()*ln2,false,12,6) <<
//			" recip_mu_t=" << to_fixpoint(1.0/mats[i].getMuT()/ln2,false,6,12) <<
//			" absfrac=" << to_fixpoint(mats[i].getAbsorbedFraction(),false,0,18) <<
//			" hg_const0=" << (hg_const0=to_fixpoint((1.0+g*g)/2.0/g,false,-2,20)) <<
//			" hg_const1=" << (hg_const1=to_fixpoint((1.0-g*g)/sqrtf(2.0*g),false,0,18)) <<
//			" hg_g=" << (hg_g=to_fixpoint(g,false,0,18)) <<
//			endl;
//
//		cout << " hg_const = " << ((hg_const0 << 36) | (hg_const1 << 18) | hg_g) << endl;

		hwmats[i] = convert_to_hw(mats[i]);
		cout << "Material [" << i << "] mu_t = " << hwmats[i].mu_t << " recip_mu_t=" << hwmats[i].recip_mu_t << " absfrac=" << hwmats[i].absfrac <<
				" hg_const=" << hwmats[i].hg_consts << endl;
	}
}
