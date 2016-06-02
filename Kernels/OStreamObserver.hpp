#ifndef OSTREAMOBSERVER_HPP_INCLUDED_
#define OSTREAMOBSERVER_HPP_INCLUDED_

#include <FullMonteSW/Kernels/KernelObserver.hpp>

class OStreamObserver : public KernelObserver {
	ostream& os;

public:
	OStreamObserver(ostream& os_) : os(os_){}

	virtual void notify_create(const Kernel&);
	virtual void notify_start(const Kernel&);
	virtual void notify_finish(const Kernel&);
	virtual void notify_result(const Kernel&,const OutputData* lr);
};


#endif
