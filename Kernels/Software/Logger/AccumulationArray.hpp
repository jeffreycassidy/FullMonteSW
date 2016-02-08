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
	QueuedAccumulatorMT(unsigned dim,unsigned queueSize=(1<<12)) : v(dim),m_queueSize(1<<10){}

	/// Move constructor: create new mutex (non-movable), move vector contents and copy default buffer size
	//QueuedAccumulatorMT(QueuedAccumulatorMT&& qa_) : m_mutex(),v(std::move(qa_.v)),m_queueSizedefault_bufsz(1<<10){}

	typedef T ElementType;

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
		WorkerThread(QueuedAccumulatorMT& master_,unsigned queueSize) : master(master_),bufsz(queueSize),
				q_start(new BufElType[bufsz]),q_curr(q_start-1),q_end(q_start+bufsz),i_last(-1){}

		/// Copy constructor deleted; inefficient since it requires allocation of buffer space
		WorkerThread(const WorkerThread&) = delete;

		/// Move constructor; steals resources and sets pointers NULL to inhibit flush/delete
		WorkerThread(WorkerThread&& acc_) : master(acc_.master),bufsz(acc_.bufsz),
				q_start(acc_.q_start),q_curr(acc_.q_curr),q_end(acc_.q_end),i_last(acc_.i_last){
			*((BufElType**)&acc_.q_start)=acc_.q_curr=NULL;
		}

		/// Flushes the buffer and deletes it
		~WorkerThread(){ commit(); delete q_start; }

		/// Returns a reference to an accumulation buffer for the given index i
		T& operator[](unsigned i){
			if (i_last != i)
			{
				if (++q_curr == q_end)
				{
					commit();
					q_curr++;
				}
				q_curr->second=0;
				q_curr->first=i_last=i;
			}
			return q_curr->second;
		}

		/// Commit all buffered updates atomically
		void commit(){
			double sum=0;
			// commit atomically to shared accumulator
			if (q_start != NULL)
			{
				master.m_mutex.lock();
				for(std::pair<unsigned,T>* p=q_start; p < min(q_curr+1,q_end); ++p)
				{
					master[p->first] += p->second;
					sum += p->second;
				}
				master.m_mutex.unlock();

				// reset state
				q_curr=q_start-1;
				i_last=-1;
			}
		}
	};

	/// Return a worker thread object
	WorkerThread get_worker(unsigned bufsz_=0) { return WorkerThread(*this,m_queueSize); };

private:
	std::mutex 	m_mutex;
	vector<T> 	v;
	unsigned 	m_queueSize=(1<<12);
};

#endif
