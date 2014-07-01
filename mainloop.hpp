#include "graph.hpp"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>
#include <boost/timer/timer.hpp>

#include "Material.hpp"

#include <pthread.h>

#include "runresults.hpp"

#include "progress.hpp"

// Manager should be able to:
//  report progress
//  start/stop/pause
//  return interim results
//  report time required
//  estimate completion time

namespace globalopts {
    extern double Npkt;         // number of packets
    extern double prwin;        // Probability of winning roulette
    extern double wmin;         // Minimum weight for roulette
    extern long Nk;             // number of packets as long int
    extern unsigned Nthread;
    extern unsigned timerinterval;
    extern unsigned randseed;
    extern string logFN;        // log filename
    extern string outpath;      // output path, defaults to working dir
    extern string psql_host;
    extern string psql_user;
    extern string psql_db;
}

// I am really sorry the worker/manager thread system here is SO ugly
// needs to be fixed!!

template<class RNG,class Logger>boost::timer::cpu_times MonteCarloLoop(unsigned long long Np,Logger& logger,const TetraMesh& mesh,const vector<Material>&,Source&);

class RunConfig;
template<class Logger,class RNG>class Manager;

template<class LoggerType,class RNG>int doOnePacket(const RunConfig& cfg,Packet pkt,
    LoggerType& logger,unsigned IDt,RNG& rng);

class RunConfig {
    public:
    const TetraMesh&        mesh;
    const vector<Material>& mat;
    const Source&           source;

    unsigned seed;
    unsigned Nthread;

    double wmin,pr_win;

    unsigned Nstep_max=10000,Nhit_max=1000;

    // default copy constructor is fine

    // prepare sources before creating run config
    RunConfig(const TetraMesh& mesh_,const vector<Material>& mat_,Source& src_,unsigned seed_=1,unsigned Nthread_=1,
        double wmin_=globalopts::wmin,double pr_win_=globalopts::prwin) :
        mesh(mesh_),
        mat(mat_),
        source(src_),
        seed(seed_),
        Nthread(Nthread_),
        wmin(wmin_),
        pr_win(pr_win_)
        { src_.prepare(mesh_); }

    template<class LoggerType,class RNG>friend int doOnePacket(const RunConfig& cfg,Packet pkt,
        LoggerType& logger,unsigned IDt,RNG& rng);
};

template<class Logger,class RNG>class Worker;

template<class Logger,class RNG>class Manager {
    boost::timer::cpu_timer t;

    protected:
    const RunConfig cfg;
    unsigned long long Npacket;

    public:

    Manager(const RunConfig& cfg_,unsigned long long Npacket_) :
        cfg(cfg_),
        Npacket(Npacket_)
        {}

    virtual void run(Logger& logger)=0;

    virtual unsigned long long getProgressCount() const { return 0; }

    virtual string getDetails() const { return ""; }

    double getProgressPercent() const  { return 100.0*double(getProgressCount())/double(Npacket); }
    double getRemainingPercent() const { return 100.0-getProgressPercent(); }
};

template<class Logger,class RNG>class Manager_MT : public Manager<Logger,RNG> {
    using Manager<Logger,RNG>::cfg;
    static void* threadStartFcn(void*);

    typedef Worker<Logger,RNG> WorkerType;

    pair<pthread_t,WorkerType*>* workers;

    public:
    virtual void run(Logger&);

    Manager_MT(const RunConfig& cfg_,unsigned long long Npacket_) :
        Manager<Logger,RNG>(cfg_,Npacket_),
        workers(new pair<pthread_t,WorkerType*>[cfg_.Nthread])
        {}

    unsigned long long getProgressCount() const {
        unsigned long long sum=0;
        for(unsigned i=0;i<cfg.Nthread;++i)
        	if (workers[i].second)
        		sum += workers[i].second->getProgressCount();
        return sum;
    }
    boost::timer::cpu_times start(Logger& logger);

    string getDetails() const {
        stringstream ss;
        for(unsigned i=0;i<cfg.Nthread && workers[i].second;++i)
        	if (workers[i].second)
        		ss << workers[i].second->getProgressCount() << " ";
        return ss.str();
    }
};

template<class Logger,class RNG>void Manager_MT<Logger,RNG>::run(Logger& logger)
{
	typedef decltype(get_worker(logger)) LoggerThread;
	LoggerThread* l[cfg.Nthread];

    cout << "Running with " << cfg.Nthread << " threads" << endl;
    // create the workers

    cout << "INFO: Nthread=" << cfg.Nthread << " alignof(WorkerType)=" << alignof(WorkerType) << " sizeof(WorkerType)=" << sizeof(WorkerType) << endl;

    void *p;
    posix_memalign(&p,32,cfg.Nthread*sizeof(WorkerType));
    if (p==NULL)
    	throw std::bad_alloc();

    for(unsigned i=0;i<cfg.Nthread;++i)
    {
    	try {
    		l[i] = new LoggerThread(get_worker(logger));
    		workers[i].second = new ((WorkerType*)p + i) WorkerType(Manager<Logger,RNG>::cfg,
    				*(l[i]),
    				Manager<Logger,RNG>::Npacket/cfg.Nthread,
    				Manager<Logger,RNG>::cfg.seed+100*i,
    				this);
            pthread_create(&workers[i].first,NULL,threadStartFcn,workers[i].second);
    	}
    	catch (string s)
    	{
    		cerr << "Thread creation failed with exception \"" << s << "\"; terminating" << endl;
    		workers[i].second=NULL;
    	}
    }

    for(unsigned i=0;i<cfg.Nthread;++i)
    {
    	if (workers[i].second)
    	{
    		pthread_join(workers[i].first,NULL);
    		cout << "Joined thread " << workers[i].first << endl;
    		workers[i].second->~WorkerType();
            delete l[i];
    	}
    }
    delete[] workers;
}

template<class Logger,class RNG>class Worker {
    public:
    const RunConfig cfg;
    unsigned long long i,Npacket;
    typedef decltype(get_worker((Logger&)*(Logger*)(0))) ThreadWorker;
    ThreadWorker& logger;
    Manager_MT<Logger,RNG>* manager;

    static const unsigned HG_BUFFERS=16;	///< Number of distinct g values to buffer
    static const unsigned HG_BUFSIZE=8;		///< Number of spin vectors to buffer

    float * hg_buffers[HG_BUFFERS];			// should be float * const but GCC whines about not being initialized
    unsigned hg_next[HG_BUFFERS];

    RNG rng;

    public:

    Worker(const RunConfig& cfg_,ThreadWorker& logger_,unsigned long long Npacket_,unsigned seed_,Manager_MT<Logger,RNG>* manager_) :
        cfg(cfg_),
        Npacket(Npacket_),
        logger(logger_),
        rng(4096,seed_),
        manager(manager_)
        {
    		void *p;
    		posix_memalign(&p,32,HG_BUFFERS*HG_BUFSIZE*4*sizeof(float));
    		for(unsigned i=0; i<HG_BUFFERS; ++i)
    		{
    			hg_buffers[i] = (float*)p + 4*HG_BUFSIZE*i;
    			hg_next[i] = HG_BUFSIZE;
    		}
        }

    ~Worker(){ free(hg_buffers[0]); }

    int doOnePacket(Packet pkt,unsigned IDt);

    inline __m128 getNextHG(unsigned matID){
    	if (hg_next[matID] == HG_BUFSIZE){
    		cfg.mat[matID].VectorHG(rng.draw_m256f8_pm1(),rng.draw_m256f8_uvect2(),hg_buffers[matID]);
    		hg_next[matID]=0;
    	}

    	return _mm_load_ps(hg_buffers[matID] + ((hg_next[matID]++)<<2));
    }

    unsigned long long getProgressCount() const { return i; }

    void start();
} __attribute__((aligned(32)));

template<class Logger,class RNG>class MgrProgressUpdate {
    const Manager<Logger,RNG>* p;
    public:
    MgrProgressUpdate(const Manager<Logger,RNG>* p_) : p(p_){}

    void operator()() const
        { cout << "\rProgress " << setw(5) << fixed << setprecision(2) << p->getProgressPercent() << "% (";
            cout << p->getDetails() << ")" << flush;
        };
};
template<class Logger,class RNG>boost::timer::cpu_times Manager_MT<Logger,RNG>::start(Logger& logger)
{
    boost::timer::cpu_timer runTimer;

    NewTimer<MgrProgressUpdate<Logger,RNG> > t(double(globalopts::timerinterval),MgrProgressUpdate<Logger,RNG>(this),false);

    if (globalopts::timerinterval > 0)
    	t.start();

    runTimer.start();
    run(logger);

    runTimer.stop();
    t.stop();
    cout << runTimer.format() << endl;
    return runTimer.elapsed();
}

template<class Logger,class RNG>void Worker<Logger,RNG>::start()
{
    pair<Packet,unsigned> tmp;
    Packet& pkt=tmp.first;
    unsigned& IDt=tmp.second;

    for(i=0;i<Npacket;++i)
    {
        tmp = cfg.source.emit(rng);
        doOnePacket(pkt,IDt);
    }
}

// thread launch function
template<class Logger,class RNG>void* Manager_MT<Logger,RNG>::threadStartFcn(void* arg)
{
    Worker<Logger,RNG>* w = (Worker<Logger,RNG>*) arg;
    try {
    	pthread_t tid=pthread_self();
    	cout << "Thread " << tid << " launched to compute " << w->Npacket << " packets" << endl;
    	w->start();
    	cout << "Thread " << tid << " terminated" << endl;
    }
    catch(string s){
    	cerr << "Caught exception string \"" << s << "\"; terminating thread" << endl;
    	throw string("Fatal worker-thread error");
    }
    return NULL;
}

template<class RNG,class Logger>boost::timer::cpu_times MonteCarloLoop(unsigned long long Np,Logger& logger,const TetraMesh& mesh,const vector<Material>& mat,Source& src)
{
    RunConfig cfg(mesh,mat,src,globalopts::randseed,globalopts::Nthread,globalopts::wmin,globalopts::prwin);

    Manager_MT<Logger,RNG> mgr(cfg,Np);

    return mgr.start(logger);
}
