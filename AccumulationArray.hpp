#ifndef ACCUMULATIONARRAY_INCLUDED
#define ACCUMULATIONARRAY_INCLUDED

#include <mutex>
#include <vector>
#include <utility>

using namespace std;

// absorption array needs to support operator[]
// items returned by operator[] need to implement +=
//
// a vector<double> is the simplest instance:
//	double& operator[]
//
// returns type double:
//	double& operator+=(double)
//
// can also do vector<HitCounter>

//! QueuedAccumulatorMT provides a thread-safe accumulator that queues accumulation requests and updates atomically using a mutex
//! It is templated on the backing store type AccumulatorMT, which must support atomic ops
/*
template<class T>class LockableVector : public std::mutex,public std::vector<T>
{
public:
	LockableVector(unsigned sz_=0) : std::vector<T>(sz_) {};
};*/

/*
class nullflush {
public:
	inline void flush(){};
};

class nulllock {
public:
	inline void lock(){};
	inline void unlock(){};
};
*/
//template<class T>VectorST<T> SimpleVector : public vector<T>,public nullflush,public nulllock {};

/** QueuedAccumulatorMT
 * Requirements:
 *   Copy-constructible (new independent buffer, reference to same master buffer)
 *   operator[] to access elements
 *   flush() to force write-out
 *
 * Automatically flushes the buffer on destruction
 *
 * Uses a LockableVector<T> as the backing store
*/

template<class T>class QueuedAccumulatorMT {
	std::mutex m;
	vector<T> v;
	unsigned default_bufsz;

protected:
	T& operator[](unsigned i){ return v[i]; }

public:
	QueuedAccumulatorMT(QueuedAccumulatorMT&& qa_) : m(),v(qa_.v),default_bufsz(qa_.default_bufsz){}
	QueuedAccumulatorMT(unsigned sz_,unsigned bufsz_=(1<<20)) : v(sz_),default_bufsz(bufsz_){}
	QueuedAccumulatorMT(const QueuedAccumulatorMT& qa_) : m(),v(qa_.v.size()),default_bufsz(qa_.default_bufsz){}

	class WorkerThread {
		/// Master accumulation array
		QueuedAccumulatorMT& master;

		/// Buffer size
		unsigned bufsz;

		/// Type of buffer element; format is (index,data)
		typedef std::pair<unsigned,T> BufElType;

		BufElType* const q_start;		///< Start of queue
		BufElType* q_curr;				///< Current queue position (start-1 => empty)
		BufElType* const q_end;			///< End of queue
		unsigned i_last;				///< Last index requested

	public:
		/// Creates a new QueuedAccumulatorMT referencing the same master list, but with a new buffer
		WorkerThread(QueuedAccumulatorMT& master_,unsigned bufsz_=0) : master(master_),bufsz(bufsz_ == 0 ? master.default_bufsz : bufsz_),
				q_start(new BufElType[bufsz_]),q_curr(q_start-1),q_end(q_start+bufsz_),i_last(-1){}

		/// References the same master but allocates a new buffer
		WorkerThread(const WorkerThread& acc_) : master(acc_.master),bufsz(acc_.bufsz),
				q_start(new BufElType[bufsz]),q_curr(q_start-1),q_end(q_start+bufsz),i_last(-1){}

		/// Move constructor; steals resources and sets pointers NULL to inhibit flush/delete
		WorkerThread(WorkerThread&& acc_) : master(acc_.master),bufsz(acc_.bufsz),
				q_start(acc_.q_start),q_curr(acc_.q_curr),q_end(acc_.q_end),i_last(acc_.i_last){
			*((BufElType**)&acc_.q_start)=acc_.q_curr=NULL;
		}

		/// Flushes the buffer and deletes it
		~WorkerThread(){ flush(); delete q_start; }

		/// Returns a reference to an accumulation buffer for the given index i
		T& operator[](unsigned i){
			if (i_last != i)
			{
				if (++q_curr == q_end)
				{
					flush();
					q_curr++;
				}
				q_curr->second=0;
				q_curr->first=i_last=i;
			}
			return q_curr->second;
		}

		/// Flush commits all current values to the array and returns to the empty state
		void flush()
		{
			// commit atomically to shared accumulator
			master.m.lock();
			for(std::pair<unsigned,T>* p=q_start; p <= q_curr; ++p)
				master[p->first] += p->second;
			master.m.unlock();

			// reset state
			q_curr=q_start-1;
			i_last=-1;
		}
	};

	/// Return a worker thread object
	WorkerThread get_worker(unsigned bufsz_=0) { return WorkerThread(*this,bufsz_==0?default_bufsz : bufsz_); };
};

#endif
