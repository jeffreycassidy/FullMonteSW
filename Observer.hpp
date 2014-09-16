#pragma once
#include <boost/timer/timer.hpp>
#include "Logger.hpp"

#include "FullMonte.hpp"


using namespace std;

class Observer {
public:
	virtual ~Observer(){}

	virtual void notify_create(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts){};
	virtual void notify_start(){};
	virtual void notify_finish(boost::timer::cpu_times t){};
	virtual void notify_result(const LoggerResults& r){};
};
