/*
 * AtomicAccumulator.hpp
 *
 *  Created on: Oct 19, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_ATOMICMULTITHREADACCUMULATOR_HPP_
#define KERNELS_SOFTWARE_LOGGER_ATOMICMULTITHREADACCUMULATOR_HPP_

#include <atomic>
#include <vector>

#include <boost/range/algorithm.hpp>

/** MultiThreadAccumulator concept
 *
 * default-constructible
 * constructible from a single argument (size)
 *
 * std::size_t size() const
 * void resize(std::size_t N)
 * void clear()
 *
 * T operator[](std::size_t i) const
 *
 * ThreadHandle	createHandle()
 *
 *
 * nested class ThreadHandle
 *
 * 		void accumulate(MTAcc&,std::size_t i,Delta d)			Performs an accumulation v[i] += d
 * 		void clear()											Clear all pending accumulations
 * 		void commit(MTAcc&)										Ensures that all previous accumulation events are reflected in master
 *
 */

template<typename Acc,typename Delta=Acc>class AtomicMultiThreadAccumulator
{
public:
	////// MultiThreadAccumulator concept requirements
	explicit AtomicMultiThreadAccumulator(std::size_t N=0);

	AtomicMultiThreadAccumulator(const AtomicMultiThreadAccumulator&)=delete;
	AtomicMultiThreadAccumulator(AtomicMultiThreadAccumulator&&)=delete;
	AtomicMultiThreadAccumulator& operator=(const AtomicMultiThreadAccumulator&)=delete;
	~AtomicMultiThreadAccumulator();

	std::size_t size() const;
	void resize(std::size_t N);
	void clear();

	Acc operator[](std::size_t i) const;

	class ThreadHandle
	{
	public:
		void accumulate(AtomicMultiThreadAccumulator& acc,std::size_t i,Delta d);
		void clear();
		void commit(AtomicMultiThreadAccumulator&);
	};

	ThreadHandle createThreadHandle();


	////// AtomicAccumulator-specific items

	unsigned long long	retryCount() const;					///< Return the number of times accumulation had to be retried
	unsigned long long	accumulationCount() const;			///< Return the number of times an event was accumulated

private:
	void accumulate(std::size_t i,Delta d);
	void commit(ThreadHandle&);

	std::atomic<Acc>*							m_values=nullptr;
	std::size_t									m_size;

	std::atomic<uint64_t>						m_retries{0};			///< Number of times accumulate() needed to loop
	std::atomic<uint64_t>						m_accumulations{0};		///< Total number of accumulations performed
};

template<typename Acc,typename Delta>AtomicMultiThreadAccumulator<Acc,Delta>::AtomicMultiThreadAccumulator(std::size_t N) :
	m_values(N > 0 ? new std::atomic<Acc>[N] : nullptr),
	m_size(N)
{
	clear();
}
//
//template<typename Acc,typename Delta>AtomicMultiThreadAccumulator<Acc,Delta>::AtomicMultiThreadAccumulator(AtomicMultiThreadAccumulator&& A) :
//		m_values(A.m_values),
//		m_size(A.m_size),
//		m_retries(A.m_retries),
//		m_accumulations(A.m_accumulations)
//{
//	A.m_values=nullptr;
//	A.m_size=0;
//	A.m_retries=A.m_accumulations=0;
//}


template<typename Acc,typename Delta>AtomicMultiThreadAccumulator<Acc,Delta>::~AtomicMultiThreadAccumulator()
{
	delete[] m_values;
}

template<typename Acc,typename Delta>std::size_t AtomicMultiThreadAccumulator<Acc,Delta>::size() const
{
	return m_size;
}

template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::resize(std::size_t i)
{
	delete[] m_values;
	m_values = new std::atomic<Acc>[i];
	if (!m_values)
		throw std::bad_alloc();
	m_size=i;
	clear();
}

template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::clear()
{
	for(unsigned i=0;i<m_size;++i)
		m_values[i] = Acc();
	m_retries=0;
	m_accumulations=0;
}

template<typename Acc,typename Delta>Acc AtomicMultiThreadAccumulator<Acc,Delta>::operator[](std::size_t i) const
{
	return m_values[i];
}

template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::accumulate(std::size_t i,Delta delta)
{
	// atomic_compare_exchange_weak(&a,&b,c) atomically:
	//	reads a
	//	compares a to b
	// 		if ==, replaces a with c and returns true
	//		if !=, replaces b with the current value of a and returns false

	// the weak variant may fail spuriously but runs faster "on some platforms"

	double oldVal = m_values[i];

	while(!std::atomic_compare_exchange_weak(&m_values[i], &oldVal, oldVal + delta))
		++m_retries;

	++m_accumulations;
}

template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::commit(ThreadHandle& H)
{
}


template<typename Acc,typename Delta>unsigned long long AtomicMultiThreadAccumulator<Acc,Delta>::retryCount() const
{
	return m_retries;
}

template<typename Acc,typename Delta>unsigned long long AtomicMultiThreadAccumulator<Acc,Delta>::accumulationCount() const
{
	return m_accumulations;
}



template<typename Acc,typename Delta>typename AtomicMultiThreadAccumulator<Acc,Delta>::ThreadHandle AtomicMultiThreadAccumulator<Acc,Delta>::createThreadHandle()
{
	return ThreadHandle();
}


template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::ThreadHandle::accumulate
	(AtomicMultiThreadAccumulator<Acc,Delta>& acc,std::size_t i,Delta d)
{
	// Just pass the accumulation through to the master to do it atomically
	acc.accumulate(i,d);
}

/// Commit is a no-op since there are no pending accumulations; we're accumulating atomically as we go
template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::ThreadHandle::commit
	(AtomicMultiThreadAccumulator<Acc,Delta>&)
{
}

/// Clear is a no-op since there are no pending accumulations; we're accumulating atomically as we go
template<typename Acc,typename Delta>void AtomicMultiThreadAccumulator<Acc,Delta>::ThreadHandle::clear()
{
}

#endif /* KERNELS_SOFTWARE_LOGGER_ATOMICMULTITHREADACCUMULATOR_HPP_ */
