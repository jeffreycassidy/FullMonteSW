#pragma once
#include <boost/timer/timer.hpp>

#include <FullMonte/Kernels/Software/Logger/Logger.hpp>

using namespace std;

class Kernel;

class KernelObserver {
public:
	virtual ~KernelObserver(){}

	virtual void notify_create(const Kernel&){};
	virtual void notify_prepare(const Kernel&){};
	virtual void notify_start(const Kernel&){};
	virtual void notify_finish(const Kernel&){};
	virtual void notify_result(const Kernel&,const LoggerResults* lr){};
};
