/*
 * HWSupport.cpp
 *
 *  Created on: Dec 18, 2014
 *      Author: jcassidy
 */

#include <boost/multiprecision/gmp.hpp>
#include "../fm-postgres/fmdbexportcase.hpp"
#include <iostream>
#include <fstream>

using namespace std;

unsigned long long to_fixpoint(double d,bool signed_,int i,int f)
{
	if (f<0)
		d /= double(1ULL << (-f));
	else
		d *= double(1ULL << f);

	unsigned long long o=0;

	if (signed_)
	{
		long long minval = -(1LL << (i+f-1));
		long long maxval = (1LL << (i+f-1))-1;

		long long t = llrint(d);

		if (t < minval)
			o=minval;
		else if (t > maxval)
			o=maxval;
		else
			o = t;
	}
	else if (d < 0)
		cout << "WARNING: Clipping negative value to zero" << endl;
	else
	{
		unsigned long long maxval = (1ULL << (i+f))-1;
		unsigned long long t = llrint(d);
		if (t > maxval)
			o=maxval;
		else
			o=t;
	}
	return o & ((1ULL << (i+f))-1ULL);
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

boost::multiprecision::number<boost::multiprecision::gmp_int> convert_to_hw(const TetraMesh& M)
{
	ofstream os("mesh.out.hex");
	boost::multiprecision::number<boost::multiprecision::gmp_int> t;

	// 4 TetraAdjacency, each:		(112)
	//		InterfaceID		(u8)
	//		TetraID			(u20)
	// 4 FaceDef, each:				(288)
	//		c (s4.14)
	//		n 3x (s0.18)
	// 1 MaterialID					  (4)
	//
	//	TOTAL						(404)

	os << "# Created by HWSupport" << endl;

	for(unsigned i=0; i<M.getNt()+1;++i)
	{
		t=0;
		const Tetra& tet = M.getTetra(i);
		for(int j=3;j >= 0;--j)
		{
			t <<= 28;
			int adjt = tet.adjTetras[j];
			if (adjt == 0)
				adjt = abs(tet.IDfs[j]) | 0x80000;
			t |= (0 << 20) | (adjt & 0xfffff);
		}

		float nx[4],ny[4],nz[4],C[4];
		_mm_store_ps(nx,tet.nx);
		_mm_store_ps(ny,tet.ny);
		_mm_store_ps(nz,tet.nz);
		_mm_store_ps(C,tet.C);

		for(int j=3;j>=0;--j)
		{
			t <<= 18;
			t |= to_fixpoint(C[j],true,4,14) & 0x3ffff;

			t <<= 18;
			t |= to_fixpoint(nz[j],true,1,17) & 0x3ffff;

			t <<= 18;
			t |= to_fixpoint(ny[j],true,1,17) & 0x3ffff;

			t <<= 18;
			t |= to_fixpoint(nx[j],true,1,17) & 0x3ffff;
		}
		t <<= 4;
		t |= tet.matID & 0xf;
		os << setw(101) << hex << setfill('0') << uppercase << t << endl;
	}

	return t;
}

int main(int argc,char **argv)
{
	boost::shared_ptr<PGConnection> conn = PGConnect();

	vector<Material> mats;

	TetraMesh* mesh = exportMesh(*conn,2);		// IDc=2 => cube_5med

	exportMaterials(*conn,2,mats);
	//exportMesh();

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

	convert_to_hw(*mesh);
}
