/** @file VectorHG.cpp Blah */

#include <stdlib.h>
#include "Material.hpp"
#include "graph.hpp"

inline void ScalarHG(const Material& m,const float* i_rand,const float* i_uvs,float *o_uv)
{
	Material::HGParams hg=m.getHGParams();
	float t=hg.one_minus_gg/(1+hg.g*i_rand[0]);

	// choose angles: HG function for component along d0, uniform circle for normal components
	float costheta = (hg.g==0 ? *i_rand : hg.recip_2g * (hg.one_plus_gg-t*t));

	o_uv[0] = costheta;
	o_uv[1] = sqrt(1-costheta*costheta);
	o_uv[2] = i_uvs[0];
    o_uv[3] = i_uvs[1];
}

#include <immintrin.h>
#include <boost/timer/timer.hpp>

inline void VectorHG2(const Material& m,const float* i_rand,const float *i_uvs,float* o)
{
	__m256 vcos,vsin;

	vcos=_mm256_load_ps(i_rand);
	__m256 ones = _mm256_set1_ps(1.0);

	__m128 tmp = m.getHGParamsSSE();

	if (_mm_comineq_ss(_mm_setzero_ps(),tmp)){		// if g != 0.0

		// insertf128_ps(__m256 a,__m128 b,int imm)	imm == 0 ? [ah,b] : [b,al]
		__m256 tmp_ext = _mm256_insertf128_ps(_mm256_castps128_ps256(tmp),tmp,1);

		// duplicate temps
		__m256 g = _mm256_shuffle_ps(tmp_ext,tmp_ext,_MM_SHUFFLE(0,0,0,0));
		__m256 recip_2g = _mm256_shuffle_ps(tmp_ext,tmp_ext,_MM_SHUFFLE(1,1,1,1));
		__m256 one_plus_gg = _mm256_shuffle_ps(tmp_ext,tmp_ext,_MM_SHUFFLE(2,2,2,2));
		__m256 one_minus_gg = _mm256_shuffle_ps(tmp_ext,tmp_ext,_MM_SHUFFLE(3,3,3,3));

		// use approximate reciprocal (error ~1e-11 from Intel)
		__m256 t = _mm256_mul_ps(one_minus_gg,_mm256_rcp_ps(_mm256_add_ps(ones,_mm256_mul_ps(g,vcos))));

		vcos = _mm256_mul_ps(recip_2g,_mm256_sub_ps(one_plus_gg,_mm256_mul_ps(t,t)));
	}

	// compute sin x = sqrt(1-cos2 x)
	vsin = _mm256_sqrt_ps(_mm256_sub_ps(ones,_mm256_mul_ps(vcos,vcos)));

	// interleave sine/cosine values
	__m256 vl = _mm256_unpacklo_ps(vcos,vsin);
	__m256 vh = _mm256_unpackhi_ps(vcos,vsin);

	//_mm256_store_ps(o,vl);
	//_mm256_store_ps(o+8,vh);

	__m256 uva = _mm256_load_ps(i_uvs);
	__m256 uvb = _mm256_load_ps(i_uvs+8);

	_mm256_store_ps(o,   _mm256_shuffle_ps(vl,uva,_MM_SHUFFLE(1,0,1,0)));
	_mm256_store_ps(o+8, _mm256_shuffle_ps(vl,uva,_MM_SHUFFLE(3,2,3,2)));
	_mm256_store_ps(o+16,_mm256_shuffle_ps(vh,uvb,_MM_SHUFFLE(1,0,1,0)));
	_mm256_store_ps(o+24,_mm256_shuffle_ps(vh,uvb,_MM_SHUFFLE(3,2,3,2)));
}

#include "graph.hpp"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

void cacheBlast(unsigned N=1<<24)
{
	unsigned *p = new unsigned[N];
	for(unsigned *t = p; t < p+N; ++t)
		*t=0;
	delete[] p;
}

void checkUnit(const float* x,unsigned N)
{
	unsigned fails=0;
	float eps = 1e-5;
	for(const float *p=x; p < x+2*N; p += 2)
		fails += fabs(p[0]*p[0]+p[1]*p[1]-1.0) > 1e-4;
	cerr << fails << "/" << N << " vectors exceed tolerance of " << eps << endl;
}

void VectorHG_Test(unsigned N=1<<20)
{
	vector<Material> mats(16);
	float g=0.9;

	for(unsigned i=0;i<16;++i)
		mats[i].setG(g);

	boost::random::mt19937 rng;
	boost::random::uniform_01<float> uni01;

	void *p;

	// Type-punning bit me here; cast &(float*) as void** and optimizer failed to update it before creating pointers below
	if (posix_memalign(&p,32,15*N*sizeof(float)))
		cerr << "Memory allocation failure" << endl;

	float * const i_rand=(float*)p;		// N random numbers on [-1,1)
	float * const o_vec=i_rand+N;		// 4N random output vectors from vector routine
	float * const o_sc=o_vec+4*N;		// 4N distributed outputs from scalar routine
	float * const o_vec2=o_sc+4*N;		// 4N distributed outputs from vector2 routine
	float * const i_uvs=o_vec2+4*N;		// 2N random unit vectors

	for(unsigned i=0;i<N;++i)
	{
		i_rand[i] = uni01(rng)*2.0-1.0;
		sincosf(uni01(rng)*2*3.141592654,i_uvs+2*i,i_uvs+2*i+1);
	}

	cacheBlast();
	cout << "Running with normal computation" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float *pi=i_rand,*po=o_sc,*ui=i_uvs; pi<i_rand+N; ++pi, po+=4, ui += 2)
			ScalarHG(mats[((unsigned long long)((void*)pi)>>5)%16],pi,ui,po);
	}

	checkUnit(o_sc,2*N);

	if (N < (1<<20))
	{
		ofstream os("hg.sc.out");
		for(const float* po=o_sc,*pi=i_rand; pi<i_rand+N; po+=4,++pi)
			os << *pi << ' ' << po[0] << ' ' << po[1] << ' ' << po[2] << ' ' << po[3] << endl;
		os.close();
	}

	cacheBlast();
	cout << "Running with vector computation" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float *pi=i_rand,*po=o_vec,*ui=i_uvs; pi<i_rand+N; pi += 8, ui += 16, po += 32)
			mats[((unsigned long long)((void*)pi)>>5)%16].VectorHG(pi,ui,po);
	}

	if(N < (1<<20))
	{
		ofstream os("hg.avx.out");
		for(const float* po=o_vec,*pi=i_rand; pi<i_rand+N; po+=4,++pi)
			os << *pi << ' ' << po[0] << ' ' << po[1] << ' ' << po[2] << ' ' << po[3] << endl;
	}
	checkUnit(o_vec,2*N);



	cacheBlast();
	cout << "Running with vector computation v2" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float *pi=i_rand,*po=o_vec2,*ui=i_uvs; pi<i_rand+N; pi += 8, ui += 16, po += 32)
			VectorHG2(mats[((unsigned long long)((void*)pi)>>5)%16],pi,ui,po);
	}

	checkUnit(o_vec2,2*N);

	if(N < (1<<20))
	{
		ofstream os("hg.avx2.out");
		for(const float* po=o_vec2,*pi=i_rand; pi<i_rand+N; po+=4,++pi)
			os << *pi << ' ' << po[0] << ' ' << po[1] << ' ' << po[2] << ' ' << po[3] << endl;
	}

}

int main(int argc,char **argv)
{
	VectorHG_Test(1<<26);
	return 0;
}
