#include <iostream>
#include <algorithm>
#include <functional>
#include <math.h>
#include <fstream>

#include <boost/timer/timer.hpp>
#include <boost/math/constants/constants.hpp>
#include <immintrin.h>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>

#define SFMT_MEXP 19937
#include "SFMT.h"
#define SFMT_INCLUDED

namespace SSE {


	#include "SFMT.c"
	#include "random.hpp"
	#include "random.cpp"
}

namespace AVX {
#include "RandomAVX.hpp"
}

using namespace std;

template<class,bool,bool>class Test_Range;
template<class T,bool ClosedL,bool ClosedU>ostream& operator<<(ostream& os,const Test_Range<T,ClosedL,ClosedU>& tr);

template<class T,bool ClosedL=true,bool ClosedU=true>class Test_Range {
	T vmin,vmax;

public:
	Test_Range(T vmin_,T vmax_) : vmin(vmin_),vmax(vmax_){}

	bool operator()(T x) const {
		return !((ClosedL ? (x >= vmin) : (x > vmin)) &&
				(ClosedU ? (x <= vmax) : (x < vmax)));
	}

	bool operator()(const T* x) const { return operator()(*x); }

	friend ostream& operator<<<>(ostream& os,const Test_Range& tr);
};

template<class T,bool ClosedL,bool ClosedU>ostream& operator<<(ostream& os,const Test_Range<T,ClosedL,ClosedU>& tr)
{
	os << " outside of range " << (ClosedL ? '[' : '(') << tr.vmin << ',' << tr.vmax << (ClosedU ? ']' : ')');
	return os;
}


template<class T>class Test_NaN {
public:
	bool operator()(T x) const { return isnan(x); }
	friend ostream& operator<<(ostream& os,const Test_NaN& tn)
		{
			return os << " have NaN values";
		}
};

// produces M outputs at a time, each output consisting of D elements of T
// returns pair<const T*,boost::cpu_times> holding result and elapsed time

template<class T>T fillval();

template<>float fillval(){ return NAN; }
template<>double fillval(){ return NAN; }
template<>unsigned fillval(){ return -1; }

template<typename T>void doTests(const T* begin, unsigned N, unsigned D)
{
}


template<typename T,typename Test,typename... Tests>void doTests(const T* begin, unsigned N, unsigned D,const Test& t,const Tests&... ts)
{
	unsigned c=0;
	for(const float *i = begin; i < begin+N*D; i += D)
		c += t(i);
	cout << "  " << setw(9) << c << ' ' << t << endl;
	doTests(begin,N,D,ts...);
}

/** Testbench for evaluating random number generation.
 *
 * Examples:
 *   randbench<float,8,2>(p,N,"Henyey-Greenstein phase function evaluation",0,"hg.avx.out",-1)
 *
 * This is a function which will generate 8 variates at each call, with two elements per variate. It therefore calls f()
 * N/8 times, generating 2N/8 output elements.
 *
 * @tparam	T			Type of base unit
 * @tparam	M			Stride when generating output data (ie. calls function N/M times)
 * @tparam	D			Number of output data units per invocation (eg. 2D unit vector = 2)
 * @tparam	F			Nullary generator function type to call
 * @tparam	Tests...	Zero or more test types to invoke
 *
 * @param	p			Pointer to memory area large enough to hold N*D elements of type T, and suitably aligned
 * @param	N			Number of elements to compute
 * @param	str			Descriptive string to print to stdout
 * @param	N_stdout	Number of elements to print to stdout (0 suppresses, -1 gives everything)
 * @param	fn			File name for file output
 * @param	N_file		Number if elements to save to file (0 suppresses, -1 gives everything)
 * @param	f			Nullary generator function to run repeatedly (useful with C++11 lambda expressions)
 * @param	ts...		Tests to run afterwards; each test must have operator()(const T*) which returns true if error
 *
 * @returns	t			boost::timer::cpu_times structure describing required run time
 */


template<class T,unsigned M,unsigned D,class F,typename... Tests>boost::timer::cpu_times randbench(T* const p,unsigned N,
		string str,int N_stdout,string fn,int N_file,F f,const Tests&... ts)
{
	boost::timer::cpu_timer t;

	cout << str << " (" << N << ')' << endl;

	// fill array with fill value to mark invalid
	fill(p,p+N*D,fillval<T>());

	// run and time
	t.start();
	for(T* o = p; o < p+N*D; o += M*D)
		f(o);
	t.stop();

	// print time
	cout << boost::timer::format(t.elapsed(),4,"%t CPU %s system") << endl << endl;

	// write to a file
	if (!fn.empty() && N_file != 0)
	{
		ofstream os(fn.c_str());
		cout << "  Writing output to " << fn << endl;
		for(const T* i=p; i < p+(N_file == -1 ? N : N_file)*D;)
		{
			os << *(i++);
			for(unsigned j=1;j<D;++i,++j)
				os << ' ' << *i;
			os << endl;
		}
	}

	if (N_stdout != 0)
	{
		for(const T* i=p; i < p+(N_stdout == -1 ? N : N_stdout)*D;)
		{
			cout << "    " << *(i++);
			for(unsigned j=1;j<D;++i,++j)
				cout << ' ' << setw(10) << *i;
			cout << endl;
		}
	}

	// run all of the tests
	doTests(p,N,D,ts...);

	return t.elapsed();
}


template<class T,unsigned D>class Test_UnitVector {
	double eps;
public:
	Test_UnitVector(double eps_) : eps(eps_){}
	Test_UnitVector(const float&  eps_) : eps(eps_){}

	Test_UnitVector(const Test_UnitVector& uv_) : eps(uv_.eps){}

	bool operator()(const T* v) const {
		double sum=0;
		for(unsigned i=0;i<D;++i)
			sum += v[i]*v[i];

		if (!isnormal(sum))
			return true;

		if (fabs(sum-1.0) > eps)
			return true;

		return false;
	}

	friend ostream& operator<<(ostream& os,const Test_UnitVector& uv){
		return os << D << "D unit vector outside tolerance of " << uv.eps;
	}
};


int main(int argc,char **argv)
{
	vector<double> v;

	AVX::RNG_SFMT_AVX rng_avx;
	SSE::RNG_SFMT rng_sse;

	unsigned N = 1<<24;			// Number of elements to draw
	unsigned D = 8;				// Size of each element

	void *p;
	posix_memalign(&p,32,N*D*sizeof(float));

	float * const o = (float*)p;
	boost::timer::cpu_times t;

	int N_file = 0;
	int N_stdout = 0;

	t = randbench<float,1,1>(o,N,"Uniform [0,1) single __m128",N_stdout,"u01.avx.out",N_file,
			[&rng_avx](float* i) -> void { _mm_store_ss(i,rng_avx.draw_m128f1_u01()); });

	t = randbench<float,1,1>(o,N,"Unit exponential __m128 scalar",N_stdout,"unitexp.avx.out",N_file,
			[&rng_avx](float* i) -> void { _mm_store_ss(i,rng_avx.draw_m128f1_exp()); },
			Test_Range<float,false,false>(0.0,std::numeric_limits<float>::infinity()));

	t = randbench<float,1,2>(o,N,"Uniform 2D unit vectors __m128",N_stdout,"uv2d.avx.out",N_file,
			[&rng_avx](float* i) -> void { __m128 t = rng_avx.draw_m128f2_uvect();
				_mm_store_ss(i,t);
				_mm_store_ss(i+1,_mm_shuffle_ps(t,t,_MM_SHUFFLE(1,1,1,1))); },
			Test_UnitVector<float,2>(1e-5),
			Test_UnitVector<float,2>(1e-6));

	t = randbench<float,1,4>(o,N,"Uniform 3D unit vectors __m128",N_stdout,"uv3d.avx.out",N_file,
				[&rng_avx](float* i) -> void { _mm_store_ps(i,rng_avx.draw_m128f3_uvect()); },
				Test_UnitVector<float,3>(1e-5),
				Test_UnitVector<float,3>(1e-6));

	t = randbench<float,1,4>(o,N,"Uniform 3D unit vectors __m128 (SSE method)",N_stdout,"uv3d.sse.out",N_file,
				[&rng_sse](float* i) -> void { _mm_store_ps(i,rng_sse.draw_m128f3_uvect()); },
				Test_UnitVector<float,3>(1e-5),
				Test_UnitVector<float,3>(1e-6));

	return 1;
}
