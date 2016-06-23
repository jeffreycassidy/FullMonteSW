#include <xmmintrin.h>
#include <pmmintrin.h>
#include <emmintrin.h>
#include "timebench.hpp"

using namespace std;

#define STRIDE_I 8
#define STRIDE_J 8
#define STRIDE_K 8

class TB_MMM : public TimeBench {
	protected:
	unsigned long N;
	float **A,**B,**C,**golden;
	virtual void allocate();
	virtual void deallocate();
	virtual void makeStimulus();
	virtual void runOnce();
    virtual bool verify() const;

	public:
	TB_MMM(unsigned long N_) : N(N_),A(NULL),B(NULL),C(NULL),golden(NULL),TimeBench(){}
};

class TB_MMM_SSE : public TB_MMM {
	protected:
	virtual void runOnce();

	public:
	TB_MMM_SSE(unsigned long N_) : TB_MMM(N_){}
};

void TB_MMM_SSE::runOnce()
{
	long unsigned ii,jj,kk;

    __m128 a0,a1,a2,a3,b0,b1,b2,b3;
    __m128 at0,at1,at2,at3,t0,t1,t2,t3,t;

    // tile it by 4's
	for(ii=0;ii<N;ii += STRIDE_I)
		for(jj=0;jj<N;jj += STRIDE_J)
			for(kk=0;kk<N;kk += STRIDE_K)
                for(unsigned i=ii;i<ii+STRIDE_I;i+=4)               // strip mine i 4x
                    for(unsigned j=jj;j<jj+STRIDE_J;j+=4)           // strip mine j 4x
                        for(unsigned k=kk;k<kk+STRIDE_K;k+=4)       // strip mine k 4x
                            // compute 4 iterations of i,j,k in here
                        {
                            // load A submatrix A[i:i+3][j:j+3], so a_n = A[i+n][j:j+3]
                            a0 = _mm_load_ps(A[i]+k);       // A[i+0][j:j+3]
                            a1 = _mm_load_ps(A[i+1]+k);
                            a2 = _mm_load_ps(A[i+2]+k);
                            a3 = _mm_load_ps(A[i+3]+k);

                            b0 = _mm_load_ps(B[k]  +j);
                            b1 = _mm_load_ps(B[k+1]+j);
                            b2 = _mm_load_ps(B[k+2]+j);
                            b3 = _mm_load_ps(B[k+3]+j);

                            t = _mm_load_ps(C[i]+j);
                            t = _mm_add_ps(t,_mm_mul_ps(b0,_mm_shuffle_ps(a0,a0,_MM_SHUFFLE(0,0,0,0))));
                            t = _mm_add_ps(t,_mm_mul_ps(b1,_mm_shuffle_ps(a0,a0,_MM_SHUFFLE(1,1,1,1))));
                            t = _mm_add_ps(t,_mm_mul_ps(b2,_mm_shuffle_ps(a0,a0,_MM_SHUFFLE(2,2,2,2))));
                            t = _mm_add_ps(t,_mm_mul_ps(b3,_mm_shuffle_ps(a0,a0,_MM_SHUFFLE(3,3,3,3))));
                            _mm_store_ps(C[i]+j,t);

                            t = _mm_load_ps(C[i+1]+j);
                            t = _mm_add_ps(t,_mm_mul_ps(b0,_mm_shuffle_ps(a1,a1,_MM_SHUFFLE(0,0,0,0))));
                            t = _mm_add_ps(t,_mm_mul_ps(b1,_mm_shuffle_ps(a1,a1,_MM_SHUFFLE(1,1,1,1))));
                            t = _mm_add_ps(t,_mm_mul_ps(b2,_mm_shuffle_ps(a1,a1,_MM_SHUFFLE(2,2,2,2))));
                            t = _mm_add_ps(t,_mm_mul_ps(b3,_mm_shuffle_ps(a1,a1,_MM_SHUFFLE(3,3,3,3))));
                            _mm_store_ps(C[i+1]+j,t);

                            t = _mm_load_ps(C[i+2]+j);
                            t = _mm_add_ps(t,_mm_mul_ps(b0,_mm_shuffle_ps(a2,a2,_MM_SHUFFLE(0,0,0,0))));
                            t = _mm_add_ps(t,_mm_mul_ps(b1,_mm_shuffle_ps(a2,a2,_MM_SHUFFLE(1,1,1,1))));
                            t = _mm_add_ps(t,_mm_mul_ps(b2,_mm_shuffle_ps(a2,a2,_MM_SHUFFLE(2,2,2,2))));
                            t = _mm_add_ps(t,_mm_mul_ps(b3,_mm_shuffle_ps(a2,a2,_MM_SHUFFLE(3,3,3,3))));
                            _mm_store_ps(C[i+2]+j,t);

                            t = _mm_load_ps(C[i+3]+j);
                            t = _mm_add_ps(t,_mm_mul_ps(b0,_mm_shuffle_ps(a3,a3,_MM_SHUFFLE(0,0,0,0))));
                            t = _mm_add_ps(t,_mm_mul_ps(b1,_mm_shuffle_ps(a3,a3,_MM_SHUFFLE(1,1,1,1))));
                            t = _mm_add_ps(t,_mm_mul_ps(b2,_mm_shuffle_ps(a3,a3,_MM_SHUFFLE(2,2,2,2))));
                            t = _mm_add_ps(t,_mm_mul_ps(b3,_mm_shuffle_ps(a3,a3,_MM_SHUFFLE(3,3,3,3))));
                            _mm_store_ps(C[i+3]+j,t);
                        }
}

void TB_MMM::runOnce()
{
	long unsigned i,j,k,ii,jj,kk;

	for(ii=0;ii<N;ii += STRIDE_I)
		for(jj=0;jj<N;jj += STRIDE_J)
			for(kk=0;kk<N;kk += STRIDE_K)
				for(i=ii;i<ii+STRIDE_I;++i)
					for(j=jj;j<jj+STRIDE_J;++j)
						for(k=kk;k<kk+STRIDE_K;++k)
							C[i][j] += A[i][k]*B[k][j];

}

void TB_MMM::allocate()
{
	A=(float**)_mm_malloc(4*N*sizeof(float*),16);
	B=A+N;
	C=B+N;
    golden=C+N;

	A[0] = (float*)_mm_malloc(4*N*N*sizeof(float),16);
	B[0] = A[0]+N*N;
	C[0] = B[0]+N*N;
    golden[0] = C[0]+N*N;

	for(unsigned i=1;i<N;++i)
	{
		A[i]=A[0]+N*i;
		B[i]=A[i]+N*N;
		C[i]=B[i]+N*N;
        golden[i]=C[i]+N*N;
	}
}

bool TB_MMM::verify() const
{
    unsigned n=0;
    float eps=1e-6;
    for(unsigned i=0;i<N;++i)
        for(unsigned j=0;j<N;++j)
            if (abs(C[i][j]-golden[i][j]) > eps)
                ++n;

    if (n > 0)
        cout << " differs in " << n << '/' << N*N << " entries" << endl;
    return n==0;
}

void TB_MMM::makeStimulus()
{
    for(unsigned i=0;i<N;++i)
        for(unsigned j=0;j<N;++j)
        {
            A[i][j]=rand();
            B[i][j]=rand();
            C[i][j]=0;
            golden[i][j]=0;
        }
    for(unsigned i=0;i<N;++i)
        for(unsigned j=0;j<N;++j)
            for(unsigned k=0;k<N;++k)
                golden[i][j] += A[i][k]*B[k][j];
}

void TB_MMM::deallocate()
{
	_mm_free(A[0]);
	_mm_free(A);
	A=B=C=golden=NULL;
}

int main(int argc,char **argv)
{
	unsigned N;
	if (argc > 1)
		N=atoi(argv[1]);
	else
		N=1024;

	TB_MMM dut(N);
	Stats st = dut.run(10);
	cout << "Non-SSE stats: " << st;

    TB_MMM_SSE dutsse(N);
    st = dutsse.run(10);
    cout << "SSE Stats: " << st;
}
