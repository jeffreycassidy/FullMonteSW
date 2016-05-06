/*
 * ThreadedMCKernelBase.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_THREADEDMCKERNELBASE_HPP_
#define KERNELS_SOFTWARE_THREADEDMCKERNELBASE_HPP_

#include "../MCKernelBase.hpp"
#include <thread>

#include <boost/random/additive_combine.hpp>

class ThreadedMCKernelBase : public MCKernelBase
{
public:
	~ThreadedMCKernelBase();

	void				threadCount(unsigned Nth)					{ Nth_=Nth;						}
	unsigned			threadCount()						const	{ return Nth_;					}

	// Final overrides: distribute the requests to the underlying threads
	virtual bool 				done() 					const 	final override;
	virtual unsigned long long 	simulatedPacketCount() 	const 	final override;

	// Final override: do parent prep and then create thread structures
	virtual void 				prepare_()						final override;

	class Thread;

	unsigned getUnsignedRNGSeed()
	{
		unsigned rnd = m_seedGenerator();
		m_seedGenerator.discard(1000);
		return rnd;
	}

private:
	virtual void				awaitFinish()					final override;
	virtual void 				start_() 						final override;

	// implemented by derived classes
	virtual Thread*				makeThread()=0;
	virtual void				parentPrepare()=0;

	static void runWorkers(ThreadedMCKernelBase* K,unsigned long long n);

	virtual void prestart()				=0;
	virtual void postfinish()			override=0;

	boost::random::ecuyer1988 					m_seedGenerator;

	unsigned 									Nth_=8;
	std::vector<ThreadedMCKernelBase::Thread*> 	m_workers;
};

#ifndef SWIG

class ThreadedMCKernelBase::Thread
{
public:
	virtual ~Thread(){}

	void start(unsigned long long N);
	bool done() const;
	void awaitFinish();

protected:
	unsigned long long 	m_nPktReq=0;
	unsigned long long 	m_nPktDone=0;

private:
	virtual void doWork()=0;

	static void threadFunction(ThreadedMCKernelBase::Thread* t);

	bool 				m_done=false;

	std::thread m_thread;

	friend class ThreadedMCKernelBase;
};

#endif

#endif /* KERNELS_SOFTWARE_THREADEDMCKERNELBASE_HPP_ */
