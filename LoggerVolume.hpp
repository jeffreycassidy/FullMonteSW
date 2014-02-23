#include "logger.hpp"
#include "AccumulationArray.hpp"

//template<class T>unsigned getID(const T& t)			{ return t.id; }		///< Returns element ID
//template<class T>double   getEnergy(const T& t)		{ return t.E; }			///< Returns total element energy (absorb/exit)
//template<class T>unsigned getMaterial(const T& t)	{ return t.matID; }		///< Returns material ID (volume only)
//template<class T>unsigned getRegion(const T& t)		{ return t.regionID; }	///< Returns region ID (volume only)
//template<class T>double   getVariance(const T& t)	{ return t.var; }		///< Returns variance estimate
//template<class T>unsigned getHits(const T& t)		{ return t.hits; }		///< Returns hit count

template<class T>class VolumeArray {
	const TetraMesh& mesh;
	vector<T> v;

public:
    VolumeArray(VolumeArray&& lv_)        : mesh(lv_.mesh),v(mesh.getNt()+1){};
    VolumeArray(const TetraMesh& mesh_)    : mesh(mesh_),v(mesh.getNt()+1){};

        /// Returns a VolumeFluenceMap for the absorption accumulated so far*/
        /** The value of asFluence determines whether it returns total energy (false) or fluence as E/V/mu_a (true) */
    void fluenceMap(VolumeFluenceMap&,const vector<Material>&,bool asFluence=true);

        /// Returns (if available from AccumulatorT) a hit map
    void hitMap(map<unsigned,unsigned long long>& m);

        /// Provides a way of summarizing to an ostream
    //friend ostream& operator<<(ostream&,VolumeArray&);
};

//! QueuedAccumulatorMT provides a thread-safe accumulator that queues accumulation requests and updates atomically using a mutex
//! It is templated on the backing store type AccumulatorMT, which must support atomic ops
//template<class T>VectorST<T> AccumulatorST : public vector<T>,public nullflush,public nulllock {};


//! Defines the base class/concept for logging volume hits in a tetrahedron

//  Each threadworker has a buffer of 1M (1024*1024) absorption events

//  When the buffer is full, it locks a mutex on the absorption map, updates, and unlocks

/*
template<class AccumulatorT=double>class LoggerVolumeMT : public LoggerVolume<AccumulatorT>,private boost::mutex {

    class ThreadWorker : private Buffer<BufElType>,public LoggerNull {
	    using Buffer<BufElType>::atBufferEnd;
        LoggerVolumeMT& parent;
        unsigned ID_last;
        virtual void atBufferEnd(const BufElType* begin,const BufElType* end){ parent.addValues(begin,end); ID_last=-1; }

        public:

        ThreadWorker(LoggerVolumeMT& parent_,unsigned N_) : Buffer<BufElType>(N_,false),parent(parent_),ID_last(-1){};
        ThreadWorker(ThreadWorker&& tw_) : Buffer<BufElType>(std::move(tw_)),parent(tw_.parent),ID_last(tw_.ID_last){}
        ~ThreadWorker() { flush(); }

        inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw)
        {
            if (ID_last == IDt)
				current->second += dw;
            else
            {
                BufElType *tmp = ((ID_last == -1) ? current : getNext());
                tmp->first=ID_last=IDt;
                tmp->second=dw;
            }
        };

        void flush()
        {
            atBufferEnd(first,current);
            current=first;
        }

        void commit()
        {
            flush();
        }
    };

    /// Return a ThreadWorker referring to this LoggerVolumeMT
    ThreadWorker getThreadWorkerInstance(unsigned)
    {
        return ThreadWorker(*this,bufsize);
    }

    // adds values to the log
    void addValues(const pair<unsigned,AccumulatorT>* p,const pair<unsigned,AccumulatorT>* p_last)
    {
        lock();
        for(; p != p_last; ++p)
	        counts[p->first] += p->second;
        unlock();
    }
};*/



/*! Basic volume logger.
 *
 * Catches eventAbsorb() calls and accumulates weight in the appropriate tetra using template argument Accumulator
 *
 * Accumulator requirements:
 * 	Support operator[] returning some type T
 * 	T must support operator+= on Weight type
 * 	Copy-constructible
 * 	Constructor of type Accumulator(unsigned size,args...)
 *
 * Accumulator WorkerThread requirements:
 * 	Copy-constructible
 *
 *
 *	Examples: vector<T>& (single-thread), QueuedAccumulatorMT (thread-safe)
 */

template<class T>class LoggerVolume;

template<class T>ostream& operator<<(ostream& os,const LoggerVolume<T>& lv);

template<class Accumulator>class LoggerVolume {
	Accumulator acc;

public:
	template<typename... Args>LoggerVolume(const TetraMesh& mesh_,Args... args) : acc(mesh_.getNt()+1,args...){}
	LoggerVolume(LoggerVolume&& lv_) : acc(std::move(lv_.acc)){}
	//LoggerVolume(const LoggerVolume& lv_) : acc(lv_.acc){}
	LoggerVolume(const LoggerVolume& lv_) = delete;

	class WorkerThread : public LoggerNull {
		typename Accumulator::WorkerThread wt;
	public:
		WorkerThread(Accumulator& parent_) : wt(parent_.get_worker()){};
		//WorkerThread(LoggerVolume& lv_) : wt(lv_.get_worker()){}
		//WorkerThread(const WorkerThread& wt_) : wt(wt_.wt){}
		WorkerThread(const WorkerThread& wt_) = delete;
		WorkerThread(WorkerThread&& wt_) : wt(std::move(wt_.wt)){}
		~WorkerThread(){ wt.commit(); }

	    inline void eventAbsorb(Point3 p,unsigned IDt,double w0,double dw)
	    	{ wt[IDt] += dw; }
	};

	typedef WorkerThread ThreadWorker;

	WorkerThread get_worker() { return WorkerThread(acc);  };

	friend ostream& operator<<<>(ostream&,const LoggerVolume&);
};
/*
template<class T>ostream& operator<<(ostream& os,const LoggerVolume<T>& lv)
{
	double t=0;
	return os << "Hello from the volume logger; total energy absorbed is " << t << endl;
}*/
