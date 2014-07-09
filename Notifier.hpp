#pragma once
#include <iostream>
#include <cxxabi.h>
#include <boost/timer/timer.hpp>

#include "FullMonte.hpp"

/*#include "Logger.hpp"
#include "LoggerEvent.hpp"*/

using namespace std;



class Observer {
protected:
	unsigned IDflight=0;
	string flightname="",flightcomment="";

public:
	Observer(unsigned IDflight_=0,string flightname="",string flightcomment="") : IDflight(IDflight_)
		{ flightstart(IDflight_,flightname,flightcomment); }

	virtual void runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned suiteid=0,unsigned caseorder=0){};
	virtual void flightstart(unsigned IDflight=0,string flightname="",string flightcomment=""){};

	virtual void runfinish(boost::timer::cpu_times t,unsigned IDrun=0){};
	virtual void flightfinish(unsigned IDflight=0){};

	/*virtual void notify_result(const ConservationCounts&){};
	virtual void notify_result(const SurfaceArray<double>&){};
	virtual void notify_result(const VolumeArray<double>&){};
	virtual void notify_result(const EventCounts&){};*/
};

class OStreamObserver : public Observer {
	ostream& os;

public:
	OStreamObserver(ostream& os_) : os(os_){}

	virtual void runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned suiteid,unsigned caseorder);
	void flightstart(unsigned IDflight=0,string flightname="",string flightcomment="");

	virtual void runfinish(boost::timer::cpu_times t,unsigned runid);
	virtual void flightfinish();
};

template<class ResultType>void handle_result(OStreamObserver& osn,const ResultType& res)
{
	osn.os << res << endl;
}

template<class ObserverType,class ResultType>void handle_result(ObserverType& obs,const ResultType& res)
{
	int status;
	cerr << "Unknown Observer/Result combination in handle_result" << endl;
	cerr << "  Observer: " << abi::__cxa_demangle(typeid(obs).name(),0,0,&status) << endl;
	cerr << "  Result:   " << abi::__cxa_demangle(typeid(res).name(),0,0,&status) << endl;
}


template<unsigned I=0,class ObserverType,typename... Ts>typename std::enable_if<(I<sizeof...(Ts)),void>::type handle_result(ObserverType& obs,const std::tuple<Ts...>& t)
{
	handle_result(obs,get<I>(t));
	handle_result<I+1>(obs,t);
}

template<unsigned I=0,class ObserverType,typename... Ts>typename std::enable_if<(I==sizeof...(Ts)),void>::type handle_result(ObserverType& obs,const std::tuple<Ts...>& t)
{}




#include "fm-postgres/fm-postgres.hpp"

class PGObserver : public Observer {
	PGConnection* dbconn=NULL;
	unsigned IDflight=0;

	unsigned createNewFlight(PGConnection* dbconn_,string="",string="");

public:
	PGObserver(PGConnection* dbconn_,string flightname_="",string flightcomment_="") :
		Observer(createNewFlight(dbconn_,flightname_,flightcomment_),flightname_,flightcomment_),
		dbconn(dbconn_){}

	virtual void runstart(const RunConfig& cfg,const RunOptions& opts,unsigned IDc,unsigned IDflight,unsigned suiteid,unsigned caseorder);

	virtual void runfinish(boost::timer::cpu_times t,unsigned runid);

};
