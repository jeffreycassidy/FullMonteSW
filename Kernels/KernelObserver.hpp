#pragma once
#include <boost/timer/timer.hpp>

using namespace std;

class Kernel;
class OutputData;

class KernelObserver {
public:
	virtual ~KernelObserver(){}

	virtual void notify_create(const Kernel&){};
	virtual void notify_prepare(const Kernel&){};
	virtual void notify_start(const Kernel&){};
	virtual void notify_finish(const Kernel&){};
	virtual void notify_result(const Kernel&,const OutputData* lr){};
};
