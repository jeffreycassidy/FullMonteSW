#ifndef ACCUMULATIONARRAY_INCLUDED
#define ACCUMULATIONARRAY_INCLUDED

#include <mutex>
#include <vector>
#include <utility>

#include "../AutoStreamBuffer.hpp"

#include <iostream>

/** QueuedMultiThreadAccumulator<Acc,Delta=Acc>
 *
 * Provides thread-safe accumulation to an array of values. Access is provided by the Queue nested class, which maintains a thread-
 * private accumulation queue. Once full, an AccumulatorQueue atomically syncs its partial results with the master protected by a
 * write mutex held by the master.
 *
 * Type arguments:
 *
 * 		Acc		The accumulator element type
 * 		Delta	The element being added (Acc += Delta must be valid)
 *
 */

template<class Acc,class Delta>class QueuedMultiThreadAccumulator
{
public:

	////// MultiThreadAccumulator requirements
	QueuedMultiThreadAccumulator(std::size_t N=0,std::size_t qSize=1024);
	QueuedMultiThreadAccumulator(QueuedMultiThreadAccumulator&&);

	QueuedMultiThreadAccumulator(const QueuedMultiThreadAccumulator&)=delete;
	QueuedMultiThreadAccumulator& operator=(const QueuedMultiThreadAccumulator&)=delete;

	~QueuedMultiThreadAccumulator();

	std::size_t size() const;
	void resize(std::size_t N);
	void clear();

	Acc operator[](std::size_t i) const;

	class ThreadHandle
	{
	public:
		ThreadHandle(std::size_t queueSize=1024);
		ThreadHandle(ThreadHandle&& A);

		ThreadHandle(const ThreadHandle&) = delete;				///< No copy constructor (could cause double-counting)
		ThreadHandle& operator=(const ThreadHandle&) = delete;	///< No assignment constructor (could cause double-counting)

		~ThreadHandle();

		void accumulate(QueuedMultiThreadAccumulator& master,std::size_t i,Delta dx);
		void commit(QueuedMultiThreadAccumulator& master);
		void clear();

	private:
		std::vector<std::pair<std::size_t,Delta>>						m_queuedValues;
		typename std::vector<std::pair<std::size_t,Delta>>::iterator 	m_curr;
	};

	ThreadHandle createThreadHandle();





	////// Queued-specific items
	void queueSize(unsigned Nq);

private:
	std::mutex 				m_writeMutex;		///< Mutex for updating
	unsigned				m_queueSize=1024;	///< Queue size when creating new ThreadHandles
	std::vector<Acc> 		m_values;			///< The values
};

template<class Acc,class Delta>QueuedMultiThreadAccumulator<Acc,Delta>::QueuedMultiThreadAccumulator(std::size_t N,std::size_t qSize) :
		m_values(N,Acc()),
		m_queueSize(qSize)
{
}

template<class Acc,class Delta>QueuedMultiThreadAccumulator<Acc,Delta>::QueuedMultiThreadAccumulator(QueuedMultiThreadAccumulator&& A) :
		m_writeMutex(),
		m_queueSize(A.m_queueSize),
		m_values(std::move(A))
{
}

template<class Acc, class Delta>QueuedMultiThreadAccumulator<Acc,Delta>::~QueuedMultiThreadAccumulator()
{
}

template<class Acc, class Delta>void QueuedMultiThreadAccumulator<Acc,Delta>::resize(std::size_t N)
{
	m_values.resize(N);
}

template<class Acc, class Delta>std::size_t QueuedMultiThreadAccumulator<Acc,Delta>::size() const
{
	return m_values.size();
}

template<class Acc, class Delta>void QueuedMultiThreadAccumulator<Acc,Delta>::clear()
{
	std::fill(m_values,Acc());
}

template<class Acc, class Delta>Acc QueuedMultiThreadAccumulator<Acc,Delta>::operator[](const std::size_t i) const
{
	return m_values[i];
}

template<class Acc,class Delta>QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::ThreadHandle
	(std::size_t queueSize) :
		m_queuedValues(queueSize)
{
	clear();
}

template<class Acc, class Delta>void QueuedMultiThreadAccumulator<Acc,Delta>::queueSize(unsigned Nq)
{
	m_queueSize=Nq;
}






template<class Acc,class Delta>QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::ThreadHandle
	(ThreadHandle&& A)
{
	unsigned i = A.m_curr - A.m_queuedValues.begin();
	m_queuedValues = std::move(A.m_queuedValues);
	m_curr = m_queuedValues.begin() + i;
}

template<class Acc, class Delta>QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::~ThreadHandle()
{
}

template<class Acc, class Delta>void QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::clear()
{
	// reset state
	m_curr=m_queuedValues.begin();
	m_curr->first = -1U;
}

template<class Acc,class Delta>void QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::accumulate
	(QueuedMultiThreadAccumulator<Acc,Delta>& master,const std::size_t i,const Delta dx)
{
	if (m_curr->first == -1U)						// hits an empty slot -> overwrite
		*m_curr = std::make_pair(i,dx);
	else if (m_curr->first == i)					// hits the same std::size_t again -> accumulate
		m_curr->second += dx;
	else											// hits a different std::size_t -> bump pointer & overwrite
	{
		++m_curr;

		if (m_curr == m_queuedValues.end())				// commit & clear if at final slot
			commit(master);

		*m_curr = std::make_pair(i,dx);					// overwrite value
	}
}

template<class Acc,class Delta>void QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::commit(QueuedMultiThreadAccumulator<Acc,Delta>& master)
{
	{
		std::unique_lock<std::mutex> L(master.m_writeMutex);

		// merge [begin, curr)
		for(auto p = m_queuedValues.begin(); p != m_curr && p->first != -1U; ++p)
		{
			if (p->first >= master.m_values.size())
			{
				std::cout << "Oops! Accumulation to element " << p->first << " exceeds bound (" << master.m_values.size() << ")" << std::endl;
				throw std::logic_error("QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle::commit - Accumulator out of bounds!");
			}
			else
				master.m_values.at(p->first) += p->second;
		}

		// if there is data in curr
		if (m_curr != m_queuedValues.end() && m_curr->first != -1U)
			master.m_values[m_curr->first] += m_curr->second;
	}
	clear();
}

template<class Acc, class Delta>typename QueuedMultiThreadAccumulator<Acc,Delta>::ThreadHandle
	QueuedMultiThreadAccumulator<Acc,Delta>::createThreadHandle()
{
	return ThreadHandle(m_queueSize);
}




#endif
