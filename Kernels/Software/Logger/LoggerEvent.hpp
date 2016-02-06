#include "Logger.hpp"
#include <mutex>

#include <FullMonte/OutputTypes/MCEventCounts.hpp>

/** LoggerEvent counts events as they happen.
 *
 */


class LoggerEvent : public LoggerNull, public MCEventCounts
{
    public:

    LoggerEvent(){ }
    LoggerEvent(const LoggerEvent&) = delete;
    LoggerEvent(LoggerEvent&& le_){ *this=le_; le_.clear(); }

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

    inline void eventAbnormal(const Packet&,unsigned,unsigned){ ++Nabnormal; }

    /// Add events contained in rhs.
    const LoggerEvent& operator+=(const LoggerEvent& rhs);

    friend class LoggerEventMT;
};


// Multi-threaded logger concept
// Must:
//	be move constructible
//	have a ThreadWorker type which
//	have a get_worker() method which returns a ThreadWorker attached to the object
//	return results
//
// Should not be:
//	copy constructible


class LoggerEventMT : public LoggerEvent, private std::mutex {
public:

	/// Default construction: init mutex
	LoggerEventMT(){}

	/// Move construction: move existing LoggerEvent, create a new mutex (not move-constructible)
	LoggerEventMT(LoggerEventMT&& le_) : LoggerEvent(std::move(le_)),std::mutex(){};

	/// Not copy-constructible
	LoggerEventMT(const LoggerEventMT&) = delete;

	/** Thread worker class for the LoggerEventMT.
	 * Implements
	 *
	 */
	class ThreadWorker : public LoggerEvent {
		LoggerEventMT& parent;
	public:
		typedef void logger_member_tag;
		/// Create a new ThreadWorker with reference to the specified parent_
		ThreadWorker(LoggerEventMT& parent_) : parent(parent_){}

		/// Move constructor, copying the reference to the parent
		ThreadWorker(ThreadWorker&& tw_) : LoggerEvent(std::move(tw_)),parent(tw_.parent){}

		/// Commit results to parent before deleting
		~ThreadWorker(){ commit(); }

		/// Atomically commit results to the parent by locking parent's mutex, adding, and then clearing local counter
		void commit()
		{
			parent.lock();
			parent += *this;
			parent.unlock();
			clear();
		}

		void eventCommit(){ commit(); }
	};


	/// Returns a new worker thread
	ThreadWorker get_worker() { return ThreadWorker(*this); }
};
