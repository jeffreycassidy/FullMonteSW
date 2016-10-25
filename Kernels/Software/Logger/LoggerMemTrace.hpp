#include <iostream>
#include <fstream>
#include <sstream>

#include "BaseLogger.hpp"

using namespace std;

typedef struct {
    unsigned IDt;
    unsigned Nabs;
} TetraRecord;

typedef struct {
    int IDf;
    unsigned Nexit;
} ExitRecord;

class LoggerMemTrace : public LoggerBase
{
    unsigned IDt_current,IDt_next;
    int IDf_last_exit;
    unsigned Nabs,Nexit;

public:

    LoggerMemTrace(string fnTetra_,string fnExit_) : fnTetra(fnTetra_),fnExit(fnExit_),os_tetra(fnTetra_),os_exit(fnExit_),
    LoggerMemTrace(const LoggerMemTrace& l_) : fnTetra(l_.fnTetra),fnExit(l_.fnExit),os_tetra(l_.fnTetra),os_exit(l_.fnExit)

    ~LoggerMemTrace();

    void eventLaunch(const Ray3 r,unsigned IDt,double w);
    void eventAbsorb(const Point3 p,unsigned IDt,double w0,double dw);
    void eventBoundary(const Point3 p,int,int,int);
    void eventInterface(const Ray3,int,unsigned);
    void eventRefract(const Point3,UVect3);

    void eventCommit(){}

    // termination events
    void eventExit(const Ray3,int,double);
};

class LoggerMemTraceMT
{
public:

	LoggerMemTraceMT(string fnroot_) : Nth(0),fnroot(fnroot_){}
	LoggerMemTraceMT(LoggerMemTraceMT&& lm_) : Nth(lm_.Nth),fnroot(std::move(lm_.fnroot)){}
	LoggerMemTraceMT(const LoggerMemTraceMT&) = delete;

    // ThreadWorker is just a single-thread instance but with a different file name
    class WorkerThread : public LoggerMemTrace {
        public:
		typedef void logger_member_tag;
        WorkerThread(string a,string b) : LoggerMemTrace(a,b){}
    };

    // thread ## writes to files tetra.trace.##.bin and exit.trace.##.bin
    WorkerThread get_worker(){
        stringstream ss_tetra,ss_exit;
        ss_tetra << fnroot << ".tetratrace." << Nth << ".bin";
        ss_exit  << fnroot << ".exittrace."  << Nth << ".bin";
        ++Nth;
        return WorkerThread(ss_tetra.str(),ss_exit.str());
    }

    typedef WorkerThread ThreadWorker;
};
