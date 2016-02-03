/*
 * Kernel.cpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#include "Kernel.hpp"

#include <FullMonte/Kernels/KernelObserver.hpp>

#include <boost/range/algorithm.hpp>
#include <memory>

using namespace std;

void Kernel::runSync()
{
	if (m_status != Idle && m_status != Finished)
		throw std::logic_error("Kernel::runSync called while status not Idle || Finished");

	updateStatus(Preparing);

	// notify observers we're preparing
	for(const auto o : m_observers)
		o->notify_prepare(*this);

	prepare_();

	// notify observers we're starting
	for(const auto o: m_observers)
		o->notify_start(*this);

	updateStatus(Running);

	start_();

	awaitFinish();
	postfinish();

	updateStatus(Finished);

	// notify observers we're done
	for(const auto o : m_observers)
		o->notify_finish(*this);

	// share the results
	for(const auto r : m_results)
		for(const auto o : m_observers)
			o->notify_result(*this,r);
}

void Kernel::startAsync()
{
	if (m_status != Idle && m_status != Finished)
		throw std::logic_error("Kernel::runSync called while status not Idle || Finished");

	// launch a thread that runs the normal synchronous routine
	m_parentThread = std::thread(std::mem_fn(&Kernel::runSync),this);

	// return as soon as it's running
	awaitStatus(Running);
}

void Kernel::finishAsync()
{
	m_parentThread.join();
}

void Kernel::updateStatus(Status status)
{
	std::unique_lock<std::mutex> lk(m_statusMutex);
	m_status=status;
	m_statusCV.notify_all();
}

void Kernel::awaitStatus(Status status)
{
	std::unique_lock<std::mutex> lk(m_statusMutex);
	m_statusCV.wait(lk, [this]{ return m_status==Running; });
}

const LoggerResults* Kernel::getResult(const std::string typeStr,const std::string opts) const
{
	auto it = boost::find_if(m_results, [&typeStr](LoggerResults* lr){ return lr && lr->getTypeString() == typeStr; });

	if (it == end(m_results))
	{
		cerr << "Failed to find results of type '" << typeStr << '\'' << endl;
		return nullptr;
	}
	else
		return *it;
}

void Kernel::clearResults()
{
	m_results.clear();
}

void Kernel::addResults(LoggerResults* r)
{
	m_results.push_back(r);
	for(auto o : m_observers)
		o->notify_result(*this,r);
}
