#include <list>
#include <array>

#include <FullMonteSW/Kernels/Event.hpp>
#include <FullMonteSW/OutputTypes/PacketPositionTrace.hpp>

#include <mutex>
#include <vector>

#include "BaseLogger.hpp"

#include "AbstractScorer.hpp"

using namespace std;

class PathScorer : public AbstractScorer
{
public:
	PathScorer();
	virtual ~PathScorer();

	typedef PacketPositionTrace::Step Step;

	class Logger;

	virtual void clear() override;

	Logger get_logger();

	std::list<OutputData*>	results() const;

	void commit(Logger& L);

private:
	std::mutex 							m_mutex;
	std::list<PacketPositionTrace*>		m_traces;
	std::size_t							m_reserveSize=16384;
};


/** Traces the photon's path into a list of vectors in memory.
 *
 */

class PathScorer::Logger : public BaseLogger
{
public:
	Logger(std::size_t reserveSize);
	Logger(Logger&& lm_) = default;
	Logger(const Logger& lm_)=delete;

	~Logger();

	typedef true_type is_logger;
	typedef std::list<PacketPositionTrace> State;

    inline void eventLaunch(AbstractScorer&,const Ray3 r,unsigned IDt,double w)
    	{ launch(r.first); }

    inline void eventAbsorb(AbstractScorer&,const Point3 p,unsigned IDt,double w0,double dw)
    	{ m_wLast=w0-dw; propagate(p,m_wLast,0.0f); }

    inline void eventRefract(AbstractScorer&,const Point3 p,UVect3& dir)
    	{ propagate(p,m_wLast,0.0f); }                	// refracted

    inline void eventReflectInternal(AbstractScorer&,const Point3 p,const UVect3 dir)
    	{ propagate(p,m_wLast,0.0f); }			// internal reflection

    inline void eventReflectFresnel(AbstractScorer&,const Point3 p,UVect3 dir)
    	{ propagate(p,m_wLast,0.0f); }         	// fresnel reflection

    // termination events
    inline void eventExit(AbstractScorer& S,const Ray3 r,int IDf,double w)
    	{ propagate(r.first,w,0.0f); terminate(static_cast<PathScorer&>(S)); };	// exited geometry

    inline void eventDie(AbstractScorer& S,double w)
    	{ terminate(static_cast<PathScorer&>(S)); }

    /// Commit is a no-op since each packet's trace will be committed when it ends
    inline void commit(AbstractScorer&){}

    void clear();

private:
    void launch(__m128 p);
    void propagate(Point3 p,float w,float t);
    void terminate(PathScorer& S);

    float 	m_length=0.0f;
    float	m_wLast=0.0f;
    __m128	m_lastPos;
    std::size_t	m_reserveSize=16384;

    std::vector<Step>	m_trace;

    friend class PathScorer;
};


