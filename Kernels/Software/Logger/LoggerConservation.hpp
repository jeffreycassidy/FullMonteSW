#include "Logger.hpp"
#include <FullMonte/Kernels/Software/Packet.hpp>
#include <mutex>

#include <FullMonte/OutputTypes/MCConservationCounts.hpp>

#include <list>


/** Single-threaded conservation tracker.
 *
 */

class LoggerConservation : public MCConservationCounts,public LoggerNull {
    public:
    inline void eventLaunch(Ray3 r,unsigned IDt,double w) { w_launch += w; };
    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw) { w_absorb += dw; };
    inline void eventExit(Ray3 r,int IDf,double w) { w_exit += w; };
    inline void eventDie(double w){ w_die += w; };
    inline void eventRouletteWin(double w0,double w){ w_roulette += w-w0; };

    inline void eventAbnormal(const Packet& pkt,unsigned,unsigned){ w_abnormal += pkt.weight(); }

    inline void eventNoHit(const Packet& pkt,const Tetra&){ w_nohit += pkt.weight(); }
    inline void eventTimeGate(const Packet& pkt){ w_time += pkt.weight(); }

    /// Initialize to zero
    LoggerConservation(){};

    /// Moves another conservation counter by copying its values and then resetting them
    LoggerConservation(LoggerConservation&& lc_) : ConservationCounts(lc_){ lc_ = LoggerConservation(); }

    /// Copy constructor is not used
    LoggerConservation(const LoggerConservation&) = delete;

    /// Return a copy of the current conservation counts
    std::list<OutputData*> results() const;

    /// Allow multi-threaded variant to access the clear() function of ConservationCounts
    friend class LoggerConservationMT;
};

std::list<OutputData*> LoggerConservation::results() const
{
	std::list<OutputData*> o;
	o.push_back(new MCConservationCounts(*this));
	return o;
}


/** Multi-threaded conservation tracker.
 * Accumulates locally using a single-threaded LoggerConservation, then merges atomically to parent using a mutex.
 */

class LoggerConservationMT : public LoggerConservation,private std::mutex {
    public:
	typedef void logger_member_tag;
	LoggerConservationMT() = default;
	LoggerConservationMT(LoggerConservationMT&& lc_) : LoggerConservation(std::move(lc_)),std::mutex(){}

	/// Should not be copy-constructible due to mutex
	LoggerConservationMT(const LoggerConservationMT&) = delete;

    class ThreadWorker : public LoggerConservation {
		/// Reference to the parent LoggerConservationMT
        LoggerConservationMT& parent;
        public:

        typedef void logger_member_tag;

        ///
        ThreadWorker(LoggerConservationMT& parent_) : parent(parent_){}

        /// Move constructor for initialization
        ThreadWorker(ThreadWorker&& tw_) : parent(tw_.parent){}

        /// No need for copy constructor
        ThreadWorker(const ThreadWorker& tw_) = delete;

        /// Commit before destroying
        ~ThreadWorker(){ commit(); }

        /// Commit atomically using mutex
        void commit()
        {
            parent.lock();
            (ConservationCounts&)parent += *this;
            parent.unlock();
            clear();
        }

        void eventCommit(){ commit(); }
    };


    /// Return a new worker
    ThreadWorker get_worker() { return ThreadWorker(*this); }
};
