#include "graph.hpp"
#include <boost/timer/timer.hpp>

#include <boost/random/additive_combine.hpp>

#include "FullMonte.hpp"
#include "Material.hpp"

#include <mutex>
#include <condition_variable>
#include <thread>

#include "runresults.hpp"

#include "Notifier.hpp"

#include "progress.hpp"

#include "SourceEmitter.hpp"


// Manager should be able to:
//  report progress
//  start/stop/pause
//  return interim results when paused
//  report time required
//  estimate completion time

class Observer;
template<class Logger,class RNG>class WorkerThread;

/** The parent class, bare minimum: run synchronously, tell if it's done or not, and give a progress count.
 *
 */

class Worker {
protected:
	vector<Observer*> obs;
	boost::timer::cpu_timer t;

	void notify_start();
	void notify_finish();
	template<unsigned I=0,typename... Ts>typename std::enable_if<(I<sizeof...(Ts)),void>::type notify_result(const std::tuple<Ts...>&) const;
	template<unsigned I=0,typename... Ts>typename std::enable_if<(I==sizeof...(Ts)),void>::type notify_result(const std::tuple<Ts...>&) const;

public:
	Worker(const vector<Observer*>& obs_ = vector<Observer*>()) : obs(obs_){}
	virtual boost::timer::cpu_times run_sync()=0;
	virtual bool done() const =0;

	virtual pair<unsigned long long,unsigned long long> getProgress() const=0;
};

template<unsigned I,typename... Ts>typename std::enable_if<(I<sizeof...(Ts)),void>::type Worker::notify_result(const std::tuple<Ts...>& t) const
{
	for(Observer* o : obs)
		handle_result(*o,get<I>(t));
	notify_result<I+1>(t);
}

template<unsigned I,typename... Ts>typename std::enable_if<(I==sizeof...(Ts)),void>::type Worker::notify_result(const std::tuple<Ts...>& t) const
{}



/** A simulation worker which can work asynchronously.
 * Adds capabilities for asynchronous start/pause/resume/wait-for-finish/stop.
 * Defaults the synchronous run capability to just start followed by wait.
 */

class AsyncWorker : public Worker {
protected:
	virtual void _impl_start_async()=0;
	virtual void _impl_finish_async()=0;
	virtual void _impl_notify_results(Observer* o)=0;

public:
	AsyncWorker(const vector<Observer*>& obs_=vector<Observer*>()) : Worker(obs_){}

	virtual boost::timer::cpu_times run_sync(){
		start_async();
		return finish_async();
	}

	void start_async(){
		t.start();
		_impl_start_async();
	}

	void pause();
	void resume();
	void stop();

	/// Block waiting for worker to finish
	boost::timer::cpu_times finish_async(){
		_impl_finish_async();
		t.stop();

		boost::timer::cpu_times elapsed = t.elapsed();

		for(Observer* o : obs)
			o->runfinish(elapsed);

		for(Observer* o : obs)
			_impl_notify_results(o);

		return elapsed;
	}
};


/** Concrete AsyncWorker implementation which manages a group of threads that share a geometry description.
 *
 */


template<class Logger,class RNG>class ThreadManager : public AsyncWorker {
	SimGeometry geom;
	RunConfig 	cfg;
	RunOptions 	opts;

	SourceEmitter<RNG>* emitter=NULL;

	Logger& logger;

	typedef decltype(get_worker(*(Logger*)(NULL))) LoggerWorker;

	LoggerWorker* loggers;
	WorkerThread<LoggerWorker,RNG>* workers;

	boost::ecuyer1988 seeds_generator;

protected:

	virtual void _impl_start_async();
	virtual void _impl_finish_async();
	virtual void _impl_notify_results(Observer* o)
	{
		auto results = __get_result_tuple2(logger);
		o->notify_result(results);
	}

    public:

    typedef decltype(__get_result_tuple2(std::declval<Logger>())) results_type;

    ThreadManager(const SimGeometry& geom_,const RunConfig& cfg_,const RunOptions& opts_,Logger& logger_,const vector<Observer*>& obs_)
    	: AsyncWorker(obs_), geom(geom_),cfg(cfg_),opts(opts_),emitter(SourceEmitterFactory<RNG>(geom_.mesh,geom_.sources)),logger(logger_),seeds_generator(opts_.randseed)
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

    	// create the workers but do not start them; this code is still single-thread
    	seeds_generator.discard(10000);
    	for(unsigned i=0;i<opts.Nthreads;++i)
    	{
    		new (loggers+i) LoggerWorker(get_worker(logger));
    		new (workers+i) WorkerThread<LoggerWorker,RNG>(emitter,geom,worker_cfg,opts,loggers[i],seeds_generator());
    		seeds_generator.discard(100);
    	}

		for(Observer* o : obs)
			o->runstart(geom,cfg,opts,geom.IDc);
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

    // TODO: Catch the error where this is called while still running (must pause first)
    //pair<boost::timer::cpu_times,results_type> results() { return make_pair(AsyncWorker::t.elapsed(),__get_result_tuple2(logger)); }
};

template<class Logger,class RNG>void ThreadManager<Logger,RNG>::_impl_start_async()
{
	// start all threads
    for(unsigned i=0;i<opts.Nthreads;++i)
    	workers[i].start_async();
}

template<class Logger,class RNG>void ThreadManager<Logger,RNG>::_impl_finish_async()
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
	const SourceEmitter<RNG> *emitter;
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

    virtual void _impl_notify_results(Observer*){}


    /// Main body which does all the work
	void thread_loop()
	{
		// lock mutex to take ownership of this worker
		std::unique_lock<std::mutex> lck(m);
		//cout << "  Thread " << t.get_id() << " paused waiting for go signal" << endl;

		pair<Packet,unsigned> tmp;
		    Packet& pkt=tmp.first;
		    unsigned& IDt=tmp.second;


		while(i<N){
			// wait for cv to notify that sim_en has been set
			while (!sim_en)
				cv.wait(lck);

			//cout << "  Thread " << t.get_id() << " received the go-ahead and is running" << endl;

			for(;i<N && sim_en; ++i)
			{
		        tmp = emitter->emit(rng);
				doOnePacket(pkt,IDt);
			}

			//if (i != N)
				//cout << "  Thread " << t.get_id() << " has been paused" << endl;
		}

		log_event(logger,Events::commit);
	}

    int doOnePacket(Packet pkt,unsigned IDt);

public:

    typedef uint32_t Seed;


    /// Temporary kludge to handle HG function generation
    inline __m128 getNextHG(unsigned matID){
    	if (hg_next[matID] == HG_BUFSIZE){
    		geom.mats[matID].VectorHG(rng.draw_m256f8_pm1(),rng.draw_m256f8_uvect2(),hg_buffers[matID]);
    		hg_next[matID]=0;
    	}
    	return _mm_load_ps(hg_buffers[matID] + ((hg_next[matID]++)<<2));
    }

    /// Note RunConfig is copied, all else is referenced from the parent
	WorkerThread(SourceEmitter<RNG>* emitter_,const SimGeometry& geom_,const RunConfig cfg_,const RunOptions& opts_,Logger& logger_,const Seed& seed_) :
		emitter(emitter_),
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

		//cout << "Thread " << t.get_id() << " created and ready to run (" << N << " packets), initially paused" << endl;
	}

	~WorkerThread(){ free(hg_buffers[0]); }

	virtual void _impl_start_async(){
		sim_en=true;
		cv.notify_all();
		//cout << "  Sending go signal" << endl;
	}

	virtual void _impl_finish_async() {
		//cout << "  Waiting for thread " << t.get_id() << " to terminate" << flush;
		t.join();
		//cout << "  DONE" << endl;
	}

	virtual bool done() const { return i==N; }
	virtual pair<unsigned long long,unsigned long long> getProgress() const { return make_pair(i,N); }

    virtual void pause(){ cerr << "ERROR: Can't pause yet" << endl; }
    virtual void stop(){  cerr << "ERROR: Can't stop yet" << endl; }
    virtual void resume(){ cerr << "ERROR: Can't resume yet" << endl; }
};
