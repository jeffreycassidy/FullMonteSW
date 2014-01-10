#include "timebench.hpp"

inline void min4floats(const float4* x,float* y)
{
    float tmin=x->f[0];
    for(unsigned i=1;i<4;++i)
        tmin=min(x->f[i],tmin);
    *y=tmin;
}

inline void min4floats_sse(const float4* x,float* y)
{
    __m128 v = _mm_load_ps((float*)x);
    __m128 min1 = _mm_min_ps(v,_mm_shuffle_ps(v,v,_MM_SHUFFLE(2,3,0,1)));
    __m128 min2 = _mm_min_ps(min1,_mm_shuffle_ps(min1,min1,_MM_SHUFFLE(0,0,2,2)));
    _mm_store_ps(y,min2);
}

void multBy2(const float* x,float *y)
{
    *y=*x * 2;
}

float4 stimGen()
{
    float4 f;
    for(unsigned i=0;i<4;++i)
        f.f[i] = double(rand())/double(RAND_MAX);
    return f;
}

// generate vectors that are close to unit (+/- 20%)

float4 nearUnitStimGen()
{
    float sum_xx=0.0;
    float4 f=stimGen();
    for(unsigned i=0;i<4;++i)
        sum_xx += f.f[i];
    float c = 1/sqrt(sum_xx)*(1+(2.0*rand()/double(RAND_MAX)-1.0)*0.2);
    for(unsigned i=0;i<4;++i)
        f.f[i] *= c;
    return f;
}

void normalize_f4(const float4* x,float4* y)
{
    float sum_xx=0.0,*fy=y->f;
    const float *fv=x->f;
    for(unsigned i=0;i<4;++i)
        sum_xx += fv[i]*fv[i];
    float c=1/sqrt(sum_xx);
    for(unsigned i=0;i<4;++i)
        fy[i] = fv[i]*c;
}

void normalize_sse(const float4* x,float4* y)
{
    __m128 v = _mm_load_ps((float*)x);
    __m128 vv = _mm_mul_ps(v,v);
    __m128  sum_vv = _mm_add_ps(vv,_mm_shuffle_ps(vv,vv,_MM_SHUFFLE(2,3,0,1)));
            sum_vv = _mm_add_ps(sum_vv,_mm_shuffle_ps(sum_vv,sum_vv,_MM_SHUFFLE(0,0,2,2)));

    __m128 c = _mm_rsqrt_ps(sum_vv);
    _mm_store_ps((float*)y,_mm_mul_ps(c,v));
}

void normalize_sse_best(const float4* x,float4* y)
{
    __m128 v = _mm_load_ps((float*)x);
    __m128 vv = _mm_mul_ps(v,v);
    __m128  sum_vv = _mm_add_ps(vv,_mm_shuffle_ps(vv,vv,_MM_SHUFFLE(2,3,0,1)));
            sum_vv = _mm_add_ps(sum_vv,_mm_shuffle_ps(sum_vv,sum_vv,_MM_SHUFFLE(0,0,2,2)));

    __m128 c = _mm_sqrt_ps(sum_vv);
    _mm_store_ps((float*)y,_mm_div_ps(v,c));
}

int main(int argc,char **argv)
{
    TimeBench<float4,float4,float4_tolerance> minbench(normalize_f4,100000000,float4_tolerance(1e-3));

    minbench.makeStimulus(nearUnitStimGen);

    Result r = minbench.run(normalize_f4);
    cout << "Errors: " << r.Nerr << '/' << r.Ntotal << endl;
    cout << "User time (us): " << r.t_user*1e6 << endl;

    r = minbench.run(normalize_sse);
//    r = minbench.run(min4floats_sse,Float4toSSE(),SSEtoScalarFloat());
    cout << "Errors: " << r.Nerr << '/' << r.Ntotal << endl;
    cout << "User time (us): " << r.t_user*1e6 << endl;
//    r = minbench.run(min4floats_sse);

    r = minbench.run(normalize_sse_best);
//    r = minbench.run(min4floats_sse,Float4toSSE(),SSEtoScalarFloat());
    cout << "Errors: " << r.Nerr << '/' << r.Ntotal << endl;
    cout << "User time (us): " << r.t_user*1e6 << endl;
}
