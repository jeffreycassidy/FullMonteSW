#include "logger.hpp"
#include <mutex>

/** LoggerEvent counts events as they happen
 *
 */

class LoggerEvent : public LoggerNull {
    public:
    unsigned long long Nlaunch,Nabsorb,Nscatter,Nbound,Ntir,Nfresnel,Nrefr,Ninterface,Nexit,Ndie,Nwin;

    void clear(){ Nlaunch=Nabsorb=Nscatter=Nbound=Ntir=Nfresnel=Nrefr=Ninterface=Nexit=Ndie=Nwin=0; }

    LoggerEvent(){ clear(); }
    LoggerEvent(const LoggerEvent&) = delete;
    LoggerEvent(LoggerEvent&& le_){ clear(); }

    inline void eventLaunch(const Ray3 r,unsigned IDt,double w){ ++Nlaunch; };   // launch new packet

    inline void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw){ ++Nabsorb; };     // packet absorbed
    inline void eventScatter(const UVect3 d0,const UVect3 d,double g){ ++Nscatter; };

    inline void eventBoundary(const Point3 p,int,int,int){ ++Nbound; };   // boundary (same material)
    inline void eventInterface(const Ray3,int,unsigned){ ++Ninterface; }; // found a material interface; possible results are:
    inline void eventRefract(const Point3,UVect3){ ++Nrefr; };                //      refracted
    inline void eventReflectInternal(const Point3,const UVect3){ ++Ntir; };        //      internal reflection
    inline void eventReflectFresnel(const Point3,UVect3){ ++Nfresnel; };         //      fresnel reflection

    // termination events
    inline void eventExit(const Ray3,int,double){ ++Nexit; };        // exited geometry
    inline void eventDie(double){ ++Ndie; };                                    // lost Russian roulette
    inline void eventRouletteWin(double,double){ ++Nwin; };                     // won roulette

    const LoggerEvent& operator+=(const LoggerEvent& rhs);
    friend ostream& operator<<(ostream&,const LoggerEvent&);
};

class LoggerEventMT : public LoggerEvent, private std::mutex {
public:
	LoggerEventMT(){}
	LoggerEventMT(const LoggerEventMT&) = delete;
	LoggerEventMT(LoggerEventMT&& le_) : LoggerEvent(std::move(le_)),std::mutex(){};

	class ThreadWorker : public LoggerEvent {
		LoggerEventMT& parent;
	public:
		ThreadWorker(LoggerEventMT& parent_) : parent(parent_){}
		ThreadWorker(ThreadWorker&& tw_) : parent(tw_.parent),LoggerEvent(std::move(tw_)){}
		~ThreadWorker(){ commit(); }

		void commit()
		{
			parent.lock();
			parent += *this;
			clear();
			parent.unlock();
		}
	};

	ThreadWorker get_worker() { return ThreadWorker(*this); }
};
