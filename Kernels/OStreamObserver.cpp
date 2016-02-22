#include "OStreamObserver.hpp"
#include <FullMonte/Kernels/Software/Logger/Logger.hpp>

#include <FullMonte/OutputTypes/OutputData.hpp>

void OStreamObserver::notify_create(const Kernel& k)
{
	os << "Run created" << endl;
}

void OStreamObserver::notify_start(const Kernel& k)
{
	os << "Run started" << endl;
}

void OStreamObserver::notify_finish(const Kernel& k)
{
	os << "Run finished" << endl;
}

void OStreamObserver::notify_result(const Kernel& k,const OutputData* lr)
{
	os << "Result available: " << lr->typeString() << endl;
}
