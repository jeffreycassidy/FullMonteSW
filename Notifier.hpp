#pragma once
#include <iostream>
#include <cxxabi.h>
#include <boost/timer/timer.hpp>
#include "Logger.hpp"

#include "FullMonte.hpp"
#include "fmdb.hpp"

using namespace std;

class Observer {
public:
	virtual ~Observer(){}

	virtual void runstart(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc){};
	virtual void runfinish(boost::timer::cpu_times t){};

	virtual void notify_result(const LoggerResults* r){};
};

class OStreamObserver : public Observer {
	ostream& os;

	static map<string,void(*)(const LoggerResults*)> op_map;

public:
	OStreamObserver(ostream& os_) : os(os_){}

	virtual void runstart(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc);
	virtual void runfinish(boost::timer::cpu_times t);
	virtual void notify_result(const LoggerResults& lr);

};



#include "fm-postgres/fm-postgres.hpp"

class PGObserver : public Observer {
	PGFlight& flight;
	PGConnection* dbconn=NULL;
	unsigned IDrun=0;

	enum { PGDataSurfaceEnergy=1,PGDataVolumeEnergy=2,PGDataSurfaceHits=3,PGDataVolumeHits=4 } PGDataTypes;

	void notifier_default(const LoggerResults*);
	void notifier_surface_energy(const LoggerResults*);
	void notifier_surface_hits(const LoggerResults*);
	void notifier_volume_hits(const LoggerResults*);
	void notifier_events(const LoggerResults*);
	void notifier_volume_energy(const LoggerResults*);

	static map<string,void(PGObserver::*)(const LoggerResults*)> op_map;


public:

	PGObserver(PGFlight& flight_) : flight(flight_),dbconn(flight_.getConnection())
		{};

	virtual void runstart(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc);
	virtual void runfinish(boost::timer::cpu_times t);

	virtual void notify_result(const LoggerResults& lr);
};
