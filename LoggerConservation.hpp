#include "logger.hpp"
#include <mutex>

// LoggerConservation - checks that energy is conserved by logging total energy:
//  launched
//  absorbed
//  expired through roulette (w_die)
//  exiting volume
//  added by roulette wins (w_roulette)

class LoggerConservation : public LoggerNull {
    double w_launch,w_absorb,w_die,w_exit,w_roulette;

    public:
    inline void eventLaunch(Ray3 r,unsigned IDt,double w) { w_launch += w; };
    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw) { w_absorb += dw; };
    inline void eventExit(Ray3 r,int IDf,double w) { w_exit += w; };
    inline void eventDie(double w){ w_die += w; };
    inline void eventRouletteWin(double w0,double w){ w_roulette += w-w0; };

    LoggerConservation() { clear(); }
    LoggerConservation(LoggerConservation&& lc_) : LoggerNull(){ clear(); };
    LoggerConservation(const LoggerConservation&) = delete;

    LoggerConservation& operator+=(const LoggerConservation& lc){
        w_launch += lc.w_launch;
        w_absorb += lc.w_absorb;
        w_die    += lc.w_die;
        w_exit   += lc.w_exit;
        w_roulette += lc.w_roulette;
        return *this;
    }

    void clear(){ w_launch=w_absorb=w_die=w_exit=w_roulette=0; }

    friend ostream& operator<<(ostream&,const LoggerConservation&);
};


// LoggerConservationMT: Multithreaded instance of conservation logger
//  just keeps one set of counters for each thread, then merges through the += operator

class LoggerConservationMT : public LoggerConservation,private std::mutex {
    public:
	LoggerConservationMT() : LoggerConservation(),std::mutex(){}
	LoggerConservationMT(LoggerConservationMT&& lc_) : LoggerConservation(std::move(lc_)),std::mutex(){}

	//LoggerConservationMT(const LoggerConservationMT& lc_) : LoggerConservation(lc_){}
	LoggerConservationMT(const LoggerConservationMT&) = delete;

    class ThreadWorker : public LoggerConservation {
        LoggerConservationMT& parent;
        public:
        ThreadWorker(LoggerConservationMT& parent_) : parent(parent_){}
        ThreadWorker(ThreadWorker&& tw_) : parent(tw_.parent){}
        ThreadWorker(const ThreadWorker& tw_) : parent(tw_.parent){}
        ~ThreadWorker(){ commit(); }

        void commit()
        {
            parent.lock();
            parent += *this;
            parent.unlock();
            clear();
        }
    };
    ThreadWorker getThreadWorkerInstance(unsigned) { return ThreadWorker(*this); }
    ThreadWorker get_worker() { return ThreadWorker(*this); }
};
