#include <immintrin.h>
#include <boost/timer/timer.hpp>
#include "graph.hpp"
#include "newgeom.hpp"
#include "Material.hpp"


#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_on_sphere.hpp>

int main(int argc,char **argv)
{
	float eps=1e-5;
	unsigned N=1<<24;

	Material mat;

	mat.setG(0.9);

	boost::random::mt19937 rng;
	boost::random::uniform_01<float> uni01;
	boost::random::uniform_on_sphere<float> unicircle(2);
	boost::random::uniform_on_sphere<float> unisphere(3);

	Packet * const ipkts = new Packet[N];					// N input packets
	Packet * const opkts = new Packet[N];					// N output packets

	void *p;
	posix_memalign(&p,32,N*4*sizeof(float));				// 4N unit vector entries (N x 4)
	__m128 * const iuvs = (__m128*)p;						// 	equivalently N __m128 vectors

	const __m128* iu=iuvs;

	const Packet *pi;
	Packet *po;

	float tmp[16] __attribute__ ((aligned(32)));
	float rnd[8]  __attribute__ ((aligned(32)));

	// MUST USE DOUBLE HERE! You get bad quantization noise if using float
	double sum_cost = 0.0f, sum_rnd=0.0f;

	// generate HG-distributed unit vectors 8 at a time (4 components ea = x32)
	for(float *p = (float*)iu; p < (float*)(iu+N); p += 32)
	{
		// get 8 random unit vectors and 8 uniform [-1,1)
		for(unsigned i=0;i<8;++i)
		{
			const vector<float> &f = unicircle(rng);
			tmp[2*i  ]=f[0];
			tmp[2*i+1]=f[1];

			sum_rnd += (rnd[i]=uni01(rng)*2.0-1.0);
		}

		// let HG function handle swizzling
		mat.VectorHG(rnd,tmp,p);

		for(unsigned i=0;i<8;++i)
			sum_cost += p[4*i];
	}

	cout << "Mean value of cos(theta)=" << sum_cost/float(N) << endl;
	cout << "Mean value of random input=" << sum_rnd/float(N) << endl;

	// generate N packets with random directions
	for(Packet *pi = ipkts; pi < ipkts+N; ++pi)
	{
		const vector<float>& f = unisphere(rng);
		pi->setDirection(f[0],f[1],f[2]);

		if(!pi->checkOrthonormalVerbose())
			cerr << "Orthonormal failure in input generation" << endl;
	}


	// time application of spins
	{
		boost::timer::auto_cpu_timer t;

		for(pi=ipkts,po=opkts; pi < ipkts+N; ++pi,++po,++iu)
			*po = pi->matspin(*pi,*iu);
	}

	// validity check
	unsigned ortho_fails=0;

	float dots[4];

	// MUST USE DOUBLE HERE! You get bad quantization noise if using float
	double sum_ddot=0.0f,sum_adot=0.0f,sum_bdot=0.0f,sum_abs_adot=0.0f,sum_abs_bdot=0.0f;

	for(pi=ipkts,po=opkts; pi < ipkts+N; ++pi,++po)
	{
		// compute dot products of output vectors with incoming basis
		_mm_store_ps(dots,pi->getDots(po->d));

		// check orthonormal output ( d.d=a.a=b.b = 1, d.a=d.b=a.b=0 )
		if(!pi->checkOrthonormal(1e-5))
			cerr << "Orthonormal failure at input" << endl;
		ortho_fails += !po->checkOrthonormalVerbose(1e-5);

		// check dots
		sum_ddot += dots[0];
		sum_adot += dots[1];
		sum_bdot += dots[2];

		sum_abs_adot += fabs(dots[1]);
		sum_abs_bdot += fabs(dots[2]);
	}

	// testbench ideas:
	//		statistical properties (total)				eg. mean d.d'
	//		individual validity (unary)					eg. orthonormal
	//		input-output correspondence (binary)		eg. input-output dots

	cout << "Found " << ortho_fails << "/" << N << " failing orthonormal test (eps=" << eps << ')' << endl;
	cout << "Mean dot with new direction: " << endl;
	cout << "  d:    " << sum_ddot/float(N) << endl;
	cout << "  a:    " << sum_adot/float(N) << endl;
	cout << "  b:    " << sum_bdot/float(N) << endl;
	cout << "  |a|:  " << sum_abs_adot/float(N) << endl;
	cout << "  |b|:  " << sum_abs_bdot/float(N) << endl;

	return 0;
}
