#include "Logger.hpp"
#include <array>
#include <fstream>
#include <iostream>

#include <FullMonte/OutputTypes/TracerStep.hpp>

using namespace std;

// Produces a trace of the photon's path: <etype, weight, px, py, pz, t>

class LoggerTracer : public LoggerNull
{
	string fn;
	ofstream os;

	float w_last=0;
	array<float,3> p_last;

private:

    inline void writeEvent(const array<float,3>& P,float w_,TracerStep::EventType e,float t_=0.0){
    	TracerStep ts;
    	ts.event=e;
    	w_last=ts.weight=w_;
    	p_last=ts.pos=P;
    	ts.t=0.0;
    	os.write((const char*)&ts,sizeof(TracerStep));
    }

    inline void writeEvent(__m128 P,float w_,TracerStep::EventType e,float t_=0.0){
    	float f[4];
    	array<float,3> a;
    	_mm_store_ps(f,P);
    	copy(f,f+3,a.begin());
    	writeEvent(a,w_,e,t_);
    }

    static bool IsAccumulate(const TracerStep& s){
    	switch(s.event){
    	case TracerStep::Launch:
    	case TracerStep::Absorb:
    	case TracerStep::Exit:
    		return true;
    	default: return false;
    	}
    }
    inline void writeEvent(const Point<3,double>& P,float w_,TracerStep::EventType e,float t_=0.0){
    	array<float,3> a;
    	a[0]=P[0];
    	a[1]=P[1];
    	a[2]=P[2];
    	writeEvent(a,w_,e,t_);
    }

    public:

	LoggerTracer(LoggerTracer&& lm_) : fn(lm_.fn),os(lm_.fn.c_str()){ lm_.os.close(); }		///< Move constructor does nothing
	LoggerTracer(const LoggerTracer& lm_)=delete;								///< Copy constructor should not exist; file name must be unique
	LoggerTracer(string fn_) : fn(fn_),os(fn_.c_str()){}
    virtual ~LoggerTracer(){ os.close(); }

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w){ writeEvent(r.first,w,TracerStep::Launch); }
    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw){ writeEvent(p,w0-dw,TracerStep::Absorb); }

    inline void eventBoundary(const Point3 p,int,int,int){ writeEvent(p,w_last,TracerStep::Boundary); }        // boundary (same material)

    inline void eventRefract(const Point3 p,UVect3& dir){ writeEvent(p,w_last,TracerStep::Refract); }                // refracted
    inline void eventReflectInternal(const Point3 p,const UVect3 dir)
    	{ writeEvent(p,w_last,TracerStep::ReflectInternal); }  //internal reflection
    inline void eventReflectFresnel(const Point3 p,UVect3 dir)
    	{ writeEvent(p,w_last,TracerStep::ReflectFresnel); }         //      fresnel reflection

    // termination events
    inline void eventExit(const Ray3 r,int IDf,double w){ writeEvent(r.first,w,TracerStep::Exit); };            // exited geometry

    inline void eventScatter(const UVect3 d0,const UVect3 d,double g){ writeEvent(p_last,w_last,TracerStep::Scatter); }

    inline void eventCommit(){}
};

class LoggerTracerMT
{
public:
	LoggerTracerMT(){}

    typedef LoggerTracer ThreadWorker;

    ThreadWorker get_worker() {
    	cout << "Creating new tracer into file " << ss.str() << endl;
    	return LoggerTracer(ss.str());
    }

    typedef ThreadWorker WorkerThread;

    result_type getResults() const { return LoggerResults(); }
};
