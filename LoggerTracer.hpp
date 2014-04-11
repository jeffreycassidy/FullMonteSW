#include "logger.hpp"
#include <array>
#include <fstream>
#include <iostream>

using namespace std;

// Produces a trace of the photon's path: <etype, weight, px, py, pz, t>

// Can infer roulette win from increasing weight to non-unity
// Can infer termination from increasing weight to unity

typedef struct TracerStep_t {
    char event;
    float weight;
    array<float,3> pos;
    float t;
} TracerStep;



class LoggerTracer : public LoggerNull
{
	string fn;
	ofstream os;
    static const char eventCodes[];


    public:
    enum EventType { Launch, Absorb, Refract, ReflectInternal, ReflectFresnel, Exit, Die };

	//LoggerTracer(){}												///< Default constructor does nothing
	LoggerTracer(LoggerTracer&& lm_) : fn(lm_.fn),os(lm_.fn.c_str()){ lm_.os.close(); }		///< Move constructor does nothing
	LoggerTracer(const LoggerTracer& lm_)=delete;					///< Copy constructor should not exist; file name must be unique
	LoggerTracer(string fn_) : fn(fn_),os(fn_.c_str()){}
	//LoggerNull& operator=(const LoggerNull& lm_){ return *this; }
    virtual ~LoggerTracer(){ os.close(); }

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w)
    	{
    	float f[4];
    	_mm_store_ps(f,r.first);
    	TracerStep ts;
    	ts.event=Launch;
    	ts.weight=w;
    	ts.pos[0]=f[0];
    	ts.pos[1]=f[1];
    	ts.pos[2]=f[2];
    	ts.t=0.0;
    	os.write((const char*)&ts,sizeof(TracerStep));
    	};

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw)
    {
    	TracerStep ts;
    	ts.event=Absorb;
    	ts.weight=w0-dw;
    	float f[4];
    	_mm_store_ps(f,p);
    	ts.pos[0]=f[0];
    	ts.pos[1]=f[1];
    	ts.pos[2]=f[2];
    	ts.t=0.0;
    	os.write((const char*)&ts,sizeof(TracerStep));
    };

//    inline void eventScatter(const UVect3 d0,const UVect3 d,double g){};

//    inline void eventBoundary(const Point3 p,int,int,int){};        // boundary (same material)

    inline void eventInterface(const Ray3,int,unsigned){};          // found a material interface; possible results are:
    inline void eventRefract(const Point3,UVect3){};                //      refracted
    inline void eventReflectInternal(const Point3,const UVect3){};  //      internal reflection
    inline void eventReflectFresnel(const Point3,UVect3){};         //      fresnel reflection

    // termination events
    inline void eventExit(const Ray3,int,double){};            // exited geometry
    inline void eventDie(double){};                                     // lost Russian roulette
    inline void eventRouletteWin(double,double){};                      // won roulette

    friend ostream& operator<<(ostream&,EventType);
};

const char LoggerTracer::eventCodes[] = { 'L','A','R','T','F','E','D','W' };

/** Display the single-character event code */
ostream& operator<<(ostream& os,LoggerTracer::EventType e)
{
    return os << LoggerTracer::eventCodes[e];
}

class LoggerTracerMT {
	unsigned i;

    public:

	LoggerTracerMT() : i(0){}

    typedef LoggerTracer ThreadWorker;
    ThreadWorker get_worker() {
    	stringstream ss;
    	ss << "tracer." << i++ << ".bin";
    	cout << "Creating new tracer into file " << ss.str() << endl;
    	return LoggerTracer(ss.str());
    }

    string getResults() const { return "<<LoggerTracer results>>"; }
};
