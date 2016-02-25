#ifndef ACCUMULATIONARRAY_INCLUDED
#define ACCUMULATIONARRAY_INCLUDED

#include <mutex>
#include <vector>
#include <utility>

using namespace std;

/** QueuedAccumulatorMT
 * Must:
 *   Copy-constructible (new independent buffer, reference to same master buffer)
 *   T& operator[](unsigned) to access elements
 *   T needs to support +=(double)
 *   flush() to force write-out
 *   Automatically flush the buffer on destruction
 *   Take first constructor argument as size
 *   Have type WorkerThread
 *   Have WorkerThread get_worker()
 *   Have typedef ElementType
 *
 * Should not be:
 * 	copy-constructible
 */

template<class T>class QueuedAccumulatorMT
{

public:
	/** Create a new QueuedAccumulatorMT.
	 * @param sz_		Accumulator size (number of elements)
	 * @param bufsz_	Buffer size (number of slots in worker thread accumulator buffer before merging with master results)
	 */
	QueuedAccumulatorMT(unsigned dim=0) : v(dim){}

	/// Move constructor: create new mutex (non-movable), move vector contents and copy default buffer size
	//QueuedAccumulatorMT(QueuedAccumulatorMT&& qa_) : m_mutex(),v(std::move(qa_.v)),m_queueSizedefault_bufsz(1<<10){}

	typedef T ElementType;

	class ThreadWorker {
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
		ThreadWorker(QueuedAccumulatorMT& master_,unsigned queueSize) : master(master_),bufsz(queueSize),
				q_start(new BufElType[bufsz]),q_curr(q_start-1),q_end(q_start+bufsz),i_last(-1){}

		/// Copy constructor deleted; inefficient since it requires allocation of buffer space
		ThreadWorker(const ThreadWorker&) = delete;

		/// Move constructor; steals resources and sets pointers NULL to inhibit flush/delete
		ThreadWorker(ThreadWorker&& acc_) : master(acc_.master),bufsz(acc_.bufsz),
				q_start(acc_.q_start),q_curr(acc_.q_curr),q_end(acc_.q_end),i_last(acc_.i_last){
			*((BufElType**)&acc_.q_start)=acc_.q_curr=NULL;
		}

		/// Flushes the buffer and deletes it
		~ThreadWorker(){ commit(); delete q_start; }

		/// Returns a reference to an accumulation buffer for the given index i
		T& operator[](unsigned i){
			if (i_last != i)
			{
				if (++q_curr == q_end)
				{
					commit();
					q_curr++;
				}
				q_curr->second=T();
				q_curr->first=i_last=i;
			}
			return q_curr->second;
		}

		void clear()
		{
			// reset state
			q_curr=q_start-1;
			i_last=-1;
		}

		/// Commit all buffered updates atomically
		void commit(){
			// commit atomically to shared accumulator
			if (q_start != NULL)
			{
				master.m_mutex.lock();
				for(std::pair<unsigned,T>* p=q_start; p < min(q_curr+1,q_end); ++p)
					master[p->first] += p->second;
				master.m_mutex.unlock();

				clear();
			}
		}
	};

	void clear()
	{
		boost::fill(v,T());
	}

	T& operator[](unsigned i){ return v[i]; }

	void resize(unsigned N){ v.resize(N,T()); }

	///< WARNING: Returns a const& but due to multi-threading access may not be safe (other threads may be writing)
	const std::vector<T>& values() const { return v; }

	/// Return a worker thread object
	ThreadWorker get_worker(unsigned qSize=0) { return ThreadWorker(*this,qSize); };

private:
	std::mutex 	m_mutex;
	vector<T> 	v;
};

#endif
