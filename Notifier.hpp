#pragma once
#include <iostream>
#include <cxxabi.h>
#include <boost/timer/timer.hpp>
#include "Logger.hpp"

#include "FullMonte.hpp"

using namespace std;

class Observer {
protected:
	unsigned IDflight=0;
	string flightname="",flightcomment="";

	virtual void _impl_notify_result(const LoggerResults&)=0;

public:
	Observer(unsigned IDflight_=0,string flightname="",string flightcomment="") : IDflight(IDflight_)
		{ flightstart(IDflight_,flightname,flightcomment); }

	virtual void runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned suiteid=0,unsigned caseorder=0){};
	virtual void flightstart(unsigned IDflight=0,string flightname="",string flightcomment=""){};

	virtual void runfinish(boost::timer::cpu_times t,unsigned IDrun=0){};
	virtual void flightfinish(unsigned IDflight=0){};

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

	virtual void runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned suiteid,unsigned caseorder);
	void flightstart(unsigned IDflight=0,string flightname="",string flightcomment="");

	virtual void runfinish(boost::timer::cpu_times t,unsigned runid);
	virtual void flightfinish();
};



#include "fm-postgres/fm-postgres.hpp"

class PGObserver : public Observer {
	PGConnection* dbconn=NULL;
	unsigned IDflight=0;

	static map<string,void(*)(PGObserver*,const LoggerResults*)> op_map;

	unsigned createNewFlight(PGConnection* dbconn_,string="",string="");

	virtual void _impl_notify_result(const LoggerResults& lr);

public:
	PGObserver(PGConnection* dbconn_,string flightname_="",string flightcomment_="") :
		Observer(createNewFlight(dbconn_,flightname_,flightcomment_),flightname_,flightcomment_),
		dbconn(dbconn_){}

	virtual void runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned IDflight,unsigned suiteid,unsigned caseorder);

	virtual void runfinish(boost::timer::cpu_times t,unsigned runid);

};

//template<typename Result>void handle_result(PGObserver&,const Result&)
//{
//	int status;
//	cout << "FAIL: PGObserver could not write a value of type " << abi::__cxa_demangle(typeid(Result).name(),0,0,&status) << endl;
//}
//
//void handle_result(PGObserver& pg,const EventCount& ec)
//{
//	cout << "(simulated database write - EventCounts" << endl << ec << endl;
//
//}
