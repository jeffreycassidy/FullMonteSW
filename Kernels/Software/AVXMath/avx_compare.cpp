#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

#include "immintrin.h"

#include <boost/timer/timer.hpp>

#define SFMT_MEXP 19937
#include "SFMT.h"

namespace avx {
#include "avx_mathfun.h"
}

namespace sse {
#define USE_SSE2
#include "sse_mathfun.h"
}

using namespace std;

int main(int argc,char **argv)
{
	boost::random::mt19937 rng;
	boost::random::uniform_01<float> uni01;

	// Alignment requires N multiple of 32
	const unsigned N = 1<<28;

	void *p;
	posix_memalign(&p,32,7*N*sizeof(float));

	float * const p_in = (float*)p;
	float * const p_sse = p_in+N;
	float * const p_avx = p_sse+2*N;

	// create N randoms on [0,2*pi)
	for(float* p=p_in; p<p_in+N; ++p)
		*p = uni01(rng)*2.0*3.141592654;

	const float *i;
	float *o;

	// bench the SSE version
	{
		boost::timer::auto_cpu_timer t;
		for(i=p_in,o=p_sse; i < p_in+N; i+=4,o+=8)
			sse::sincos_ps(*(__m128*)i,(__m128*)o,(__m128*)(o+4));
	}

	// bench the AVX version
	cout << "Alignof(__m256)=" << alignof(__m256) << endl;


	// bench the AVX version
	cout << "AVX sincos_ps" << endl;
	{
		boost::timer::auto_cpu_timer t;
		for(i=p_in,o=p_avx; i < p_in+N; i+=8,o+=16)
			avx::sincos_ps(*(__m256*)i,o,o+8);
	}

	// bench the SSE version
	cout << "Original SSE sincos_ps" << endl;
	{
		boost::timer::auto_cpu_timer t;
		for(i=p_in,o=p_sse; i < p_in+N; i+=4,o+=8)
			sse::sincos_ps(*(__m128*)i,(__m128*)o,(__m128*)(o+4));
	}


	ofstream os("sincos.avx.txt");
	for(unsigned k=0;k<N;++k)
	{
		unsigned k4 = (k/4)*8  + (k%4);
		unsigned k8 = (k/8)*16 + (k%8);

		if (k<64)
			cout << setprecision(6) << setw(10) << p_in[k] << ' ' << setw(10) << p_sse[k4] << ' ' << setw(10) << p_sse[k4+4] << ' ' << setw(10) << p_avx[k8] << ' ' << setw(10) << p_avx[k8+8] << endl;
		if (k<(1<<20))
			os << setprecision(6) << setw(10) << p_in[k] << ' ' << setw(10) << p_sse[k4] << ' ' << setw(10) << p_sse[k4+4] << ' ' << setw(10) << p_avx[k8] << ' ' << setw(10) << p_avx[k8+8] << endl;
	}


	// create N randoms on [0,2*pi)
	for(float* p=p_in; p<p_in+N; ++p)
		*p = uni01(rng);

	// bench the AVX version
	cout << "AVX log_ps" << endl;
	{
		boost::timer::auto_cpu_timer t;
		for(i=p_in,o=p_avx; i < p_in+N; i+=8,o+=8)
			_mm256_store_ps(o,avx::log_ps(*(__m256*)i));
	}

	// bench the SSE version
	cout << "Original SSE log_ps" << endl;
	{
		boost::timer::auto_cpu_timer t;
		for(i=p_in,o=p_sse; i < p_in+N; i+=4,o+=4)
			_mm_store_ps(o,sse::log_ps(*(__m128*)i));
	}


	os.open("log.avx.txt",ios_base::out);
	for(unsigned k=0;k<N;++k)
	{
		if (k<64)
			cout << setprecision(6) << setw(10) << p_in[k] << ' ' << setw(10) << p_sse[k] << ' ' << setw(10) << p_avx[k] << ' ' << endl;
		if (k<(1<<20))
			os << setprecision(6) << setw(10) << p_in[k] << ' ' << setw(10) << p_sse[k] << ' ' << setw(10) << p_avx[k] << ' ' << endl;
	}
}
