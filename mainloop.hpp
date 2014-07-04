#include "graph.hpp"
#include <boost/timer/timer.hpp>

#include "FullMonte.hpp"
#include "Material.hpp"

#include <mutex>
#include <condition_variable>
#include <thread>

#include "runresults.hpp"

#include "progress.hpp"

// Manager should be able to:
//  report progress
//  start/stop/pause
//  return interim results when paused
//  report time required
//  estimate completion time


/*
template<class Logger,class RNG>class MgrProgressUpdate {
    const Manager<Logger,RNG>* p;
    public:
    MgrProgressUpdate(const Manager<Logger,RNG>* p_) : p(p_){}

    void operator()() const
        { cout << "\rProgress " << setw(5) << fixed << setprecision(2) << p->getProgressPercent() << "% (";
            cout << p->getDetails() << ")" << flush;
        };
};*/

template<class Logger,class RNG>class WorkerThread;

/** The parent class, bare minimum: run synchronously, tell if it's done or not, and give a progress count.
 *
 */

class Worker {

public:
	virtual boost::timer::cpu_times run_sync()=0;
	virtual bool done() const =0;

	virtual pair<unsigned long long,unsigned long long> getProgress() const=0;
};



/** A simulation worker which can work asynchronously.
 * Adds capabilities for asynchronous start/pause/resume/wait-for-finish.
 * Defaults the synchronous run capability to just start followed by wait.
 */

class AsyncWorker : public Worker {

public:

	virtual boost::timer::cpu_times run_sync(){
		boost::timer::cpu_timer t;
		t.start();
		start_async();
		finish_async();
		t.stop();
		return t.elapsed();
	}

	virtual void start_async()=0;
	virtual void pause()=0;
	virtual void resume()=0;
	virtual void stop()=0;
	virtual void finish_async()=0;			///< Block waiting for worker to finish
};


/** Concrete AsyncWorker implementation which manages a group of threads that share a geometry description.
 *
 */


template<class Logger,class RNG>class ThreadManager : public AsyncWorker {
	SimGeometry geom;
	RunConfig 	cfg;
	RunOptions 	opts;

	Logger& logger;

	typedef decltype(get_worker(*(Logger*)(NULL))) LoggerWorker;

	LoggerWorker* loggers;
	WorkerThread<LoggerWorker,RNG>* workers;

    public:

    ThreadManager(const SimGeometry& geom_,const RunConfig& cfg_,const RunOptions& opts_,Logger& logger_)
    	: geom(geom_),cfg(cfg_),opts(opts_),logger(logger_)
    {
    	// allocate aligned space for the workers
    	posix_memalign((void**)&loggers, 64, opts.Nthreads*sizeof(LoggerWorker));
    	posix_memalign((void**)&workers, 64, opts.Nthreads*sizeof(WorkerThread<LoggerWorker,RNG>));

    	// throw exception if allocation fails
    	if (!loggers || !workers)
    		throw std::bad_alloc();

    	// create configuration for workers (each gets 1/N as many packets to sim)
    	RunConfig worker_cfg(cfg);
    	worker_cfg.Npackets /= opts.Nthreads;

    	// create the workers but do not start them
    	for(unsigned i=0;i<opts.Nthreads;++i)
    	{
    		new (loggers+i) LoggerWorker(get_worker(logger));
    		new (workers+i) WorkerThread<LoggerWorker,RNG>(geom,worker_cfg,opts,loggers[i],i);
    	}
    }

    ~ThreadManager(){ free(loggers); free(workers); }

    pair<unsigned long long,unsigned long long> getProgress() const {
        unsigned long long sum_completed=0,sum_total=0;
        unsigned long long completed,total;
        for(const WorkerThread<LoggerWorker,RNG>* w=workers; w<workers+opts.Nthreads; ++w)
        {
        	tie(completed,total) = w->getProgress();
        	sum_completed += completed;
        	sum_total     += total;
        }
        return make_pair(sum_completed,sum_total);
    }

    void start_async();						///< Asynchronous simulation start
    void finish_async();					///< Asynchronous simulation finish

    /// Poll all workers for status
    virtual bool done() const {
    	for(unsigned i=0;i<opts.Nthreads;++i)
    		if (!workers[i].done())
    			return false;
    	return true;
    }

    virtual void pause(){ cerr << "ERROR: Can't pause yet" << endl; }
    virtual void stop(){  cerr << "ERROR: Can't stop yet" << endl; }
    virtual void resume(){ cerr << "ERROR: Can't resume yet" << endl; }
};

template<class Logger,class RNG>void ThreadManager<Logger,RNG>::start_async()
{
    cout << "Launching sim: " << endl;
    cout << geom << endl << cfg << endl << opts << endl;

    for(unsigned i=0;i<opts.Nthreads;++i)
    	workers[i].start_async();
}

template<class Logger,class RNG>void ThreadManager<Logger,RNG>::finish_async()
{
	// wait for all threads to finish
    for(unsigned i=0;i<opts.Nthreads;++i)
    	workers[i].finish_async();
}




/** Represents a thread-safe simulation worker.
 * Reasonably lightweight, copies only the configuration (options & geometry are referenced).
 *
 */


template<class Logger,class RNG> class WorkerThread : public AsyncWorker {
	const SimGeometry& 	geom;
	const RunConfig    	cfg;
	const RunOptions& 	opts;

	mutex m;								///< Mutex giving ownership of this thread
	condition_variable cv;					///< Handles notification of simulation-enable

	bool sim_en=false;						///< Simulation enable
	thread t;								///< The thread which does the computing
	Logger& logger;							///< Logger object

	// Loop counters
	unsigned long long N=0;			///< Total number of items to do
	unsigned long long i=0;			///< Number of items completed

    RNG rng;

    static const unsigned HG_BUFFERS=20;	///< Number of distinct g values to buffer
    static const unsigned HG_BUFSIZE=8;		///< Number of spin vectors to buffer

    float * hg_buffers[HG_BUFFERS];			// should be float * const but GCC whines about not being initialized
    unsigned hg_next[HG_BUFFERS];


    /// Main body which does all the work
	void thread_loop()
	{
		// lock mutex to take ownership of this worker
		std::unique_lock<std::mutex> lck(m);
		cout << "  Thread " << t.get_id() << " paused waiting for go signal" << endl;

		pair<Packet,unsigned> tmp;
		    Packet& pkt=tmp.first;
		    unsigned& IDt=tmp.second;


		while(i<N){
			// wait for cv to notify that sim_en has been set
			while (!sim_en)
				cv.wait(lck);

			cout << "  Thread " << t.get_id() << " received the go-ahead and is running" << endl;

			for(;i<N && sim_en; ++i)
			{
		        tmp = geom.sources[0]->emit(rng);
				doOnePacket(pkt,IDt);
			}

			if (i != N)
				cout << "  Thread " << t.get_id() << " has been paused" << endl;
		}

		log_event(logger,Events::commit);
	}

    int doOnePacket(Packet pkt,unsigned IDt);

public:

    /// Temporary kludge to handle HG function generation
    inline __m128 getNextHG(unsigned matID){
    	if (hg_next[matID] == HG_BUFSIZE){
    		geom.mats[matID].VectorHG(rng.draw_m256f8_pm1(),rng.draw_m256f8_uvect2(),hg_buffers[matID]);
    		hg_next[matID]=0;
    	}
    	return _mm_load_ps(hg_buffers[matID] + ((hg_next[matID]++)<<2));
    }

    /// Note RunConfig is copied, all else is referenced from the parent
	WorkerThread(const SimGeometry& geom_,const RunConfig cfg_,const RunOptions& opts_,Logger& logger_,unsigned seed_) :
		geom(geom_),
		cfg(cfg_),
		opts(opts_),
		logger(logger_),
		N(cfg_.Npackets),
		rng(4096,seed_)
	{
		// Allocate HG buffers (currently necessary to get correct alignment; future GCC may relieve the requirement)
		//
		void *p;
		posix_memalign(&p,32,HG_BUFFERS*HG_BUFSIZE*4*sizeof(float));
		for(unsigned i=0; i<HG_BUFFERS; ++i)
		{
			hg_buffers[i] = (float*)p + 4*HG_BUFSIZE*i;
			hg_next[i] = HG_BUFSIZE;
		}

		// create thread waiting for condition variable; should be last thing done
		t=std::thread(mem_fn(&WorkerThread<Logger,RNG>::thread_loop),this);

		cout << "Thread " << t.get_id() << " created and ready to run (" << N << " packets), initially paused" << endl;
	}

	~WorkerThread(){ free(hg_buffers[0]); }

	virtual void start_async(){
		sim_en=true;
		cv.notify_all();
		cout << "  Sending go signal" << endl;
	}

	virtual void finish_async() {
		cout << "  Waiting for thread " << t.get_id() << " to terminate" << endl;
		t.join();
		cout << "  Done" << endl;
	}

	virtual bool done() const { return i==N; }
	virtual pair<unsigned long long,unsigned long long> getProgress() const { return make_pair(i,N); }

    virtual void pause(){ cerr << "ERROR: Can't pause yet" << endl; }
    virtual void stop(){  cerr << "ERROR: Can't stop yet" << endl; }
    virtual void resume(){ cerr << "ERROR: Can't resume yet" << endl; }
};
