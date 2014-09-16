#pragma once
#include <iostream>
#include <cxxabi.h>
#include <boost/timer/timer.hpp>
#include "Logger.hpp"

#include "FullMonte.hpp"
#include "fmdb.hpp"

using namespace std;

#include "Observer.hpp"

class PGObserver : public Observer {
	PGFlight& flight;
	PGConnection* dbconn=NULL;
	unsigned IDrun=0;
	unsigned IDcase=0;

	enum { PGDataSurfaceEnergy=1,PGDataVolumeEnergy=2,PGDataSurfaceHits=3,PGDataVolumeHits=4 } PGDataTypes;

	void notifier_default(const LoggerResults*);
	void notifier_surface_energy(const LoggerResults*);
	void notifier_surface_hits(const LoggerResults*);
	void notifier_volume_hits(const LoggerResults*);
	void notifier_events(const LoggerResults*);
	void notifier_volume_energy(const LoggerResults*);

	static map<string,void(PGObserver::*)(const LoggerResults*)> op_map;


public:

	PGObserver(PGFlight& flight_,unsigned IDcase_) : flight(flight_),dbconn(flight_.getConnection()),IDcase(IDcase_)
		{};

	void setCaseID(unsigned IDcase_){ IDcase=IDcase_; }

	virtual void notify_create(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts);
	virtual void notify_start();
	virtual void notify_finish(boost::timer::cpu_times t);
	virtual void notify_result(const LoggerResults& lr);
};
