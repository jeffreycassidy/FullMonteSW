#pragma once
#include "Observer.hpp"

class OStreamObserver : public Observer {
	ostream& os;

public:
	OStreamObserver(ostream& os_) : os(os_){}

	virtual void notify_create(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts);
	virtual void notify_start();
	virtual void notify_finish(boost::timer::cpu_times t);
	virtual void notify_result(const LoggerResults& lr);
};
