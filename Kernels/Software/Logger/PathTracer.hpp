#include <list>
#include <array>

#include <FullMonteSW/Kernels/Event.hpp>
#include <FullMonteSW/OutputTypes/PacketPositionTrace.hpp>

#include <cmath>
#include <mutex>
#include "BaseLogger.hpp"

using namespace std;

/** Traces the photon's path into a list of vectors in memory.
 *
 */

class PathTracer : public BaseLogger
{
public:
	PathTracer(){};
	PathTracer(PathTracer&& lm_) = default;
	PathTracer(const PathTracer& lm_)=delete;

	typedef true_type is_logger;
	typedef std::list<PacketPositionTrace> State;

    virtual ~PathTracer();

    inline void eventLaunch(AbstractScorer&,const Ray3 r,unsigned IDt,double w)
    	{ writeEvent(KernelEvent::Launch,r.first,w,NAN); }

    inline void eventAbsorb(AbstractScorer&,const Point3 p,unsigned IDt,double w0,double dw)
    	{ writeEvent(KernelEvent::Absorb,p,w0-dw,NAN); }

    inline void eventBoundary(AbstractScorer&,const Point3 p,int,int,int)
    	{ writeEvent(KernelEvent::Boundary,p,m_lastW,NAN); }        		// boundary (same material)

    inline void eventRefract(AbstractScorer&,const Point3 p,UVect3& dir)
    	{ writeEvent(KernelEvent::Refract,p,m_lastW,NAN); }                	// refracted

    inline void eventReflectInternal(AbstractScorer&,const Point3 p,const UVect3 dir)
    	{ writeEvent(KernelEvent::ReflectInternal,p,m_lastW,NAN); }			// internal reflection

    inline void eventReflectFresnel(AbstractScorer&,const Point3 p,UVect3 dir)
    	{ writeEvent(KernelEvent::ReflectFresnel,p,m_lastW,NAN); }         	// fresnel reflection

    // termination events
    inline void eventExit(AbstractScorer&,const Ray3 r,int IDf,double w)
    	{ writeEvent(KernelEvent::Exit,r.first,w,NAN); };            		// exited geometry

    inline void eventScatter(AbstractScorer&,const UVect3 d0,const UVect3 d,double g)
    	{ writeEvent(KernelEvent::Scatter,m_lastPos,m_lastW,NAN); }

    /// Commit can be a no-op since all threads have their own private list
    inline void commit(AbstractScorer&)
    	{
    	}

    inline void clear()
    {
    	m_traces.clear();
    }

private:
    void writeEvent(KernelEvent::Type e,Point3 p,float w,float t);

    float 	m_pathLength=0.0f;
    Point3 	m_lastPos=Point3{NAN,NAN,NAN};
    float	m_lastW=1.0f;

    std::vector<PacketPositionTrace::Step>	m_currentTrace;
    std::list<PacketPositionTrace>			m_traces;
};

PathTracer::~PathTracer()
{
}

inline void PathTracer::writeEvent(KernelEvent::Type e,const Point3 p,float w,float t)
{
	if (e == KernelEvent::Launch)
	{
		m_currentTrace.clear();
		m_pathLength = 0.0f;

		m_currentTrace.emplace_back();
	}
	else
	{
		Point3 delta = p-m_lastPos;

		float f[4];
		_mm_store_ps(f,delta*delta);

		m_pathLength += std::sqrt(f[0]+f[1]+f[2]);
	}

	if (isPacketTermination(e))
		m_traces.emplace_back(std::move(m_currentTrace));

	m_lastPos = p;
	m_lastW = w;
}
//
//class LoggerTracerMT
//{
//public:
//	LoggerTracerMT(){}
//
//	typedef true_type is_logger;
//
//    typedef PathTracer ThreadWorker;
//
//    ThreadWorker get_worker()
//    {
//    	return PathTracer();
//    }
//
//    inline void eventClear(){  }
//
//    void merge(const std::list<PacketPositionTrace>& T);
//
//private:
//    std::mutex						m_resultMutex;
//    std::list<PacketPositionTrace>	m_results;
//};

//inline void LoggerTracerMT::merge(const std::list<PacketPositionTrace>& T)
//{
//	std::lock(m_resultMutex);
//	for(const auto& l : T.state())
//		m_results.push_back(l);
//}
