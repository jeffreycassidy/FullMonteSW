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
	startAsync();
	awaitFinish();
}

void Kernel::startAsync()
{
	for(const auto o : m_observers)
		o->notify_prepare(*this);

	prepare_();

	for(const auto o: m_observers)
		o->notify_start(*this);

	start_();
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
