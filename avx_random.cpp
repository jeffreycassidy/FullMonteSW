#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/math/constants/constants.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <immintrin.h>

#include <boost/timer/timer.hpp>

#define SFMT_MEXP 19937
#include "SFMT.h"

namespace SSE {
#include "SFMT.c"
#include "random.hpp"
#include "random.cpp"
}

namespace AVX {
#include "RandomAVX.hpp"
}

#include <stdlib.h>

// countIf


// Individual
// Range check (eg. unit vectors are unit, uniform range, not NaN, etc)
// Print
// Save to file

// Aggregate
// Distribution check (Octave)
// Mean check
// Stddev check

using namespace std;

AVX::RNG_SFMT_AVX avx(1024,1);

SSE::RNG_SFMT sse(1024,1);

int main(int argc,char **argv)
{
	const unsigned long N = (1<<24);
	void *p;

	ofstream os;

	posix_memalign(&p,32,N*11*sizeof(float));

	float * const avx_log  = (float*)p;
	float * const avx_uv2d = avx_log+N;
	float * const avx_pm1 = avx_uv2d+2*N;
	float * const avx_u01 = avx_pm1+N;
	float * const sse_uv2d = avx_u01+N;
	__m256i * const justrand = (__m256i*)(sse_uv2d+2*N);
	float * const sse_exp = ((float*)justrand) + N;
	float * const avx_exp = sse_exp + N;

	unsigned fails=0;

	// test U01
	cout << "just plain ol' randoms, 8 at a time" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(__m256i* o=justrand; o< justrand+N/8; ++o)
			_mm256_store_si256(o,avx.draw());
	}

	/*for(const float* o=avx_u01; o<avx_u01+N; ++o)
	{
		//			if (o < avx_u01+32)
		//cout << *o << endl;
		//fails += (*o < 0 || *o >= 1.0);
	}*/

	// cout << fails << '/' << N << " out of range" << endl;



	// test U01
	/*cout << "just plain ol' randoms, 1 at a time" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float * o = justrand_singleint; o< justrand_singleint+N; ++o)
			*o = avx.draw_ui32();
	}*/

	/*for(const float* o=avx_u01; o<avx_u01+N; ++o)
	{
		//			if (o < avx_u01+32)
		//cout << *o << endl;
		//fails += (*o < 0 || *o >= 1.0);
	}*/

	// cout << fails << '/' << N << " out of range" << endl;



	// test U01
	cout << "U[0,1) randoms" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float* o=avx_u01; o< avx_u01+N; ++o)
			_mm_store_ss(o,avx.draw_m128f1_u01());
	}


	for(const float* o=avx_u01; o<avx_u01+N; ++o)
	{
		//if (o < avx_u01+32)
			//cout << *o << endl;
		fails += (*o < 0 || *o >= 1.0);
	}

	cout << fails << '/' << N << " out of range" << endl;


	// test +/- 1 randoms
	cout << "U[-1,1) randoms" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float* o=avx_pm1; o< avx_pm1+N; ++o)
			_mm_store_ss(o,avx.draw_m128f1_pm1());
	}

	fails=0;
	for(const float* o=avx_pm1; o<avx_pm1+N; ++o)
	{
		//if (o < avx_pm1+32)
			//cout << *o << endl;
		fails += (*o < -1.0 || *o >= 1.0);
	}

	cout << fails << '/' << N << " out of range" << endl;


	// test uv2d
	cout << "2D unit vector randoms" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float* o=avx_uv2d; o< avx_uv2d+2*N; o+=2)
		{
			__m128 uv = avx.draw_m128f2_uvect();
			_mm_store_ss(o,  uv);
			_mm_store_ss(o+1,_mm_shuffle_ps(uv,uv,_MM_SHUFFLE(1,1,1,1)));
		}
	}

	fails=0;
	for(const float* o=avx_uv2d; o<avx_uv2d+2*N; o+=2)
	{
		float len=sqrt(o[0]*o[0] + o[1]*o[1]);
		if (o < avx_uv2d+32)
			cout << '(' << setw(10) << o[0] << ',' << setw(10) << o[1] << ") ||.|| = " << len << endl;
		fails += fabs(len-1.0) > 1e-6;
	}
	cout << fails << '/' << N << " out of range" << endl;


	// test uv2d
	cout << "SSE 2D unit vector randoms" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float* o=sse_uv2d; o< sse_uv2d+2*N; o+=2)
		{
			__m128 uv = sse.draw_m128f2_uvect();
			_mm_store_ss(o,  uv);
			_mm_store_ss(o+1,_mm_shuffle_ps(uv,uv,_MM_SHUFFLE(1,1,1,1)));
		}
	}

	fails=0;
	for(const float* o=sse_uv2d; o<sse_uv2d+2*N; o+=2)
	{
		float len=sqrt(o[0]*o[0] + o[1]*o[1]);
		if (o < sse_uv2d+32)
			cout << '(' << setw(10) << o[0] << ',' << setw(10) << o[1] << ") ||.|| = " << len << endl;
		fails += fabs(len-1.0) > 1e-6;
	}
	cout << fails << '/' << N << " out of range" << endl;





	// test SSE exponential dist
	os.open("unitexp.sse.out");
	cout << "SSE exponential dist" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float* o=sse_exp; o< sse_exp+N; ++o)
		{
			__m128 uv = sse.draw_m128f1_log_u01();
			_mm_store_ss(o,  uv);
		}
	}

	fails=0;
	for(const float* o=sse_exp; o<sse_exp+N; ++o)
	{
		if (o < sse_exp+32)
			cout << setw(10) << *o << endl;
		os << *o << endl;
		fails += *o <= 0.0 || isnan(*o);
	}
	cout << fails << '/' << N << " out of range" << endl;
	os.close();






	// test AVX exponential dist
	os.open("unitexp.avx.out");
	cout << "AVX unit exponential distribution" << endl;
	{
		boost::timer::auto_cpu_timer t;

		for(float* o=avx_exp; o< avx_exp+N; ++o)
		{
			__m128 uv = avx.draw_m128f1_exp();
			_mm_store_ss(o,  uv);
		}
	}

	fails=0;
	for(const float* o=avx_exp; o<avx_exp+N; ++o)
	{
		if (o < avx_exp+32)
			cout << setw(10) << o[0] << endl;
		os << *o << endl;
		fails += *o <= 0 || isnan(*o);
	}
	cout << fails << '/' << N << " out of range" << endl;
	os.close();

	free(p);
}
