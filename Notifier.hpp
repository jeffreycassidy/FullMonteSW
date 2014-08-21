#pragma once
#include <iostream>
#include <cxxabi.h>
#include <boost/timer/timer.hpp>
#include "Logger.hpp"

#include "FullMonte.hpp"
#include "fmdb.hpp"

using namespace std;

class Observer {

	virtual void _impl_notify_result(const LoggerResults&)=0;

public:
	Observer(){}

	virtual ~Observer(){}

	virtual void runstart(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc){};
	virtual void runfinish(boost::timer::cpu_times t){};

	template<unsigned I=0,typename... Ts>typename std::enable_if<(I<sizeof...(Ts)),void>::type notify_result(const std::tuple<Ts...>& t)
	{
		this->_impl_notify_result(get<I>(t));
		notify_result<I+1>(t);
	}
	template<unsigned I=0,typename... Ts>typename std::enable_if<(I==sizeof...(Ts)),void>::type notify_result(const std::tuple<Ts...>& t)
		{}
};

class OStreamObserver : public Observer {
	ostream& os;

	static map<string,void(*)(const LoggerResults*)> op_map;

	void _impl_notify_result(const LoggerResults& lr);

public:
	OStreamObserver(ostream& os_) : os(os_){}

	virtual void runstart(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc);
	virtual void runfinish(boost::timer::cpu_times t);
};



#include "fm-postgres/fm-postgres.hpp"

class PGObserver : public Observer {
	PGFlight& flight;
	PGConnection* dbconn=NULL;
	unsigned IDrun=0;

	enum { PGDataSurfaceEnergy=1,PGDataVolumeEnergy=2,PGDataSurfaceHits=3,PGDataVolumeHits=4 } PGDataTypes;

	void notifier_default(const LoggerResults*);
	void notifier_surface_energy(const LoggerResults*);
	void notifier_events(const LoggerResults*);
	void notifier_volume_energy(const LoggerResults*);

	static map<string,void(PGObserver::*)(const LoggerResults*)> op_map;

	virtual void _impl_notify_result(const LoggerResults& lr);

public:

	PGObserver(PGFlight& flight_) : flight(flight_),dbconn(flight_.getConnection())
		{};

	virtual void runstart(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts,unsigned IDc);
	virtual void runfinish(boost::timer::cpu_times t);

};
