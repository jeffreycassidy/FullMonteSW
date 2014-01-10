#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <inttypes.h>

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <boost/timer/timer.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>

#include "../newgeom.hpp"

// set up golden reference

using namespace std;

void cacheWipe(unsigned N=10000000,bool=false);

class float4 {
    public:
    float f[4];

    float4(){}
    float4(__m128 v){ _mm_store_ps(f,v); }
    operator __m128() const { return _mm_load_ps(f); }
};

// reference case

template<class A,class B>class default_comp {
    public:
    bool operator()(const A& a,const B& b) const { return a==b; }
};

class float4_tolerance {
    float eps;
    public:
    float4_tolerance(float eps_=1e-6) : eps(eps_*eps_){}

    bool operator()(const float4& a,const float4&b) const {
        float sum_xx=0.0,diff;
        for(unsigned i=0;i<4;++i)
        {
            diff = a.f[i]-b.f[i];
            sum_xx += diff*diff;
        }
        return sum_xx < eps;
    }
};

template<class T,class U=T>class Identity {
    public:
    typedef T input_type;
    typedef U result_type;
    U operator()(T t) const { return t; }
};

class SSEtoUnitVector2 {
    public:
    typedef __m128 input_type;
    typedef UnitVector<2,double> result_type;
    result_type operator()(input_type x)
        { float f[4]; double d[2]; _mm_store_ps(f,x); d[0]=f[0]; d[1]=f[1]; return UnitVector<2,double>(d); }
};

class SSEtoScalarFloat {
    public:
    typedef __m128  input_type;
    typedef float   result_type;

    result_type operator()(input_type x){ float f; _mm_store_ss(&f,x); return f; }
};

class Float4toSSE {
    public:
    typedef float4  input_type;
    typedef __m128  result_type;

    result_type operator()(input_type x){ return _mm_load_ps((float*)&x); }
};

typedef struct {
    double t_wall,t_user,t_sys;
    unsigned Ntotal,Nerr;
} Result;

template<class InputType,class OutputType,class Comp=default_comp<OutputType,OutputType> >class TimeBench {
    public:
    typedef void(*func_type)(const InputType*,OutputType*);
    typedef InputType   type;
    typedef OutputType  result_type;

    private:
    unsigned            N;
    InputType* const    idata;
    OutputType* const   odata_ref;
    func_type           f_ref;
    Comp                C;

	public:

//    TimeBench(func_type F,unsigned N_,Comp C_=default_comp<OutputType,OutputType>()) :
    TimeBench(func_type F,unsigned N_,Comp C_) :
        N(N_),idata(new InputType[N_]),odata_ref(new OutputType[N_]),f_ref(F),C(C_){ }

    template<class G>void makeStimulus(G g){
        generate_n(idata,N,g);
        for(unsigned i=0;i<N;++i)
            f_ref(idata+i,odata_ref+i);
        }

    Result run(func_type,bool=false) const;

    template<class IMap,class OMap>Result run(void(*)(const typename IMap::result_type*,typename OMap::input_type*),IMap imap,OMap omap,bool=false) const;
};

template<class InputType,class OutputType,class Comp>template<class IMap,class OMap>Result TimeBench<InputType,OutputType,Comp>::run(void(*fcn)(const typename IMap::result_type*,typename OMap::input_type*),IMap imap,OMap omap=Identity<OutputType>(),bool print) const
{
    typedef typename IMap::result_type IType;
    typedef typename OMap::input_type  OType;
    Result res;
    boost::timer::cpu_timer t;
    IType *idata_conv=new IType[N];
    OType *odata_test=new OType[N];


    // convert input data into the new type
    transform(idata,idata+N,idata_conv,imap);
    cacheWipe();

    // start timer
    t.start();

    for(unsigned i=0;i<N;++i)
        fcn(idata_conv+i,odata_test+i);

    // stop timer
    t.stop();

    boost::timer::cpu_times e=t.elapsed();

    res.Nerr=0;

    for(unsigned i=0;i<N;++i)
    {
        if (print)
            cout << idata[i] << " ==> " << odata_test[i] << " Expecting " << odata_ref[i] << endl;
        if (!C(odata_ref[i],omap(odata_test[i])))
            ++res.Nerr;
    }

    // pack up results
    res.t_wall=e.wall*1e-9;
    res.t_sys=e.system*1e-9;
    res.t_user=e.user*1e-9;
    res.Ntotal=N;

    delete[] idata_conv;
    delete[] odata_test;

    return res;
}

template<class InputType,class OutputType,class CompType>Result TimeBench<InputType,OutputType,CompType>::run(func_type fcn,bool print) const
{
    Result res;
    boost::timer::cpu_timer t;
    result_type         *odata_test=new OutputType[N];

    // clear the cache
    cacheWipe();

    // start timer
    t.start();

    // run the test function
    for(unsigned i=0;i<N;++i)
        fcn(idata+i,odata_test+i);

    // stop timer
    t.stop();

//    for(unsigned i=0;i<N && print; ++i)
//      cout << idata[i] << " ==> " << odata_test[i] << endl;

    boost::timer::cpu_times e=t.elapsed();

    res.Nerr=0;

    bool iserr = false;

    for(unsigned i=0;i<N;++i)
    {
        iserr = !C(odata_ref[i],odata_test[i]);
//        if (iserr && res.Nerr < 100)
//            cout << "expecting " << odata_ref[i] << endl << "      got " << odata_test[i] << endl;
        res.Nerr += iserr;
    }

    if (res.Nerr > 100)
        cout << " ... and " << res.Nerr-100 << " more " << endl;

    // pack up results
    res.t_wall=e.wall*1e-9;
    res.t_sys=e.system*1e-9;
    res.t_user=e.user*1e-9;
    res.Ntotal=N;

    delete[] odata_test;

    return res;
}

