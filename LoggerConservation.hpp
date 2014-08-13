#include "Logger.hpp"
#include "Packet.hpp"
#include <mutex>

/** Tracks overall photon-packet conservation statistics.
 * Provides basic facilities to initialize, copy, accumulate, and print to ostream. *
 */

struct ConservationCounts {
    double w_launch;			///< Amount of energy launched (generally 1.0 * Npackets)
    double w_absorb;			///< Amount of energy absorbed
    double w_die;				///< Amount of energy terminated in roulette
    double w_exit;				///< Amount of energy exiting
    double w_roulette;			///< Amount of energy added by winning roulette
    double w_abnormal;			///< Amount of energy terminated due to abnormal circumstances
    double w_time;				///< Amount of energy terminated due to time gate expiry
    double w_nohit;				///< Amount of energy terminated for failure to find an intersecting face

    /// Initialize to zero
    ConservationCounts(){ clear(); }

    /// Copy values from another ConservationCounts element
    ConservationCounts(const ConservationCounts& cc_) = default;

    /// Clear all elements to zero
    void clear(){ w_launch=w_absorb=w_die=w_exit=w_roulette=w_time=w_abnormal=w_nohit=0; }

    /// Add another ConservationCounts
    ConservationCounts& operator+=(const ConservationCounts&);

    /// Output to ostream&
    friend ostream& operator<<(ostream&,const ConservationCounts&);
};


/** Single-threaded conservation tracker.
 *
 */

class LoggerConservation : public ConservationCounts,public LoggerNull {
    public:
    inline void eventLaunch(Ray3 r,unsigned IDt,double w) { w_launch += w; };
    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw) { w_absorb += dw; };
    inline void eventExit(Ray3 r,int IDf,double w) { w_exit += w; };
    inline void eventDie(double w){ w_die += w; };
    inline void eventRouletteWin(double w0,double w){ w_roulette += w-w0; };

    inline void eventAbnormal(const Packet& pkt,unsigned,unsigned){ w_abnormal += pkt.w; }

    inline void eventNoHit(const Packet& pkt,const Tetra&){ w_nohit += pkt.w; }
    inline void eventTimeGate(const Packet& pkt){ w_time += pkt.w; }

    /// Initialize to zero
    LoggerConservation() =default;

    /// Moves another conservation counter by copying its values and then resetting them
    LoggerConservation(LoggerConservation&& lc_) : ConservationCounts(lc_){ lc_.clear(); }

    /// Copy constructor is not used
    LoggerConservation(const LoggerConservation&) = delete;

    typedef ConservationCounts result_type;

    /// Return a copy of the current conservation counts
    result_type getResults() const { return *this; }

    /// Allow multi-threaded variant to access the clear() function of ConservationCounts
    friend class LoggerConservationMT;
};


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
    };

    /// Return a new worker
    ThreadWorker get_worker() { return ThreadWorker(*this); }
};
