#include "Notifier.hpp"
#include "LoggerEvent.hpp"
#include "LoggerConservation.hpp"

#include "LoggerVolume.hpp"
#include "LoggerSurface.hpp"

void OStreamObserver::runstart(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,unsigned IDc)
{
	os << "Run started; problem definition:" << endl << geom << cfg << endl << opts << endl;
}

void OStreamObserver::runfinish(boost::timer::cpu_times t)
{
	os << "Run finished, elapsed time: " << format(t) << endl;
}

void PGObserver::runstart(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,unsigned IDc)
{
    IDrun = flight.newRun(geom,cfg,opts,IDc);
    cout << "  PGObserver attached to run " << IDrun << endl;
}

void PGObserver::runfinish(boost::timer::cpu_times t)			// ignores runid argument in favour of internally stored one
{
    //const char* log = res.log_stdout.getSize() > 0 ? (const char*)res.log_stdout.getPtr() : "";
    //const char* err = res.log_stderr.getSize() > 0 ? (const char*)res.log_stderr.getPtr() : "";
	//const char *err = "";
	//const char *log = "";

		// write results to database
	dbconn->execParams("INSERT INTO runresults(runid,exitcode,t_wall,t_user,t_system) VALUES ($1,$2,$3,$4,$5)",
			boost::tuples::make_tuple(IDrun,0,double(t.wall)*1e-9,double(t.user)*1e-9,double(t.system)*1e-9));
//		dbconn->execParams("INSERT INTO logtext(runid,stderr_text,stdout_text) VALUES ($1,$2,$3);",boost::tuples::make_tuple(
//					runid,err,log));
}

void OStreamObserver::_impl_notify_result(const LoggerResults& lr)
{
	lr.summarize(cout);
}

void PGObserver::_impl_notify_result(const LoggerResults& lr)
{
	string type=lr.getTypeString();
	map<string,void(PGObserver::*)(const LoggerResults*)>::const_iterator it=op_map.find(type);
	cout << "PGObserver received notification for result " << type;
	if (it == op_map.end())
	{
		cout << " (no writer found)" << endl;
		this->notifier_default(&lr);
	}
	else if ( void(PGObserver::*handler_ptr)(const LoggerResults*) = it->second)
	{
		cout << endl;
		(this->*handler_ptr)(&lr);
	}
	else
		cout << "Found it, but it's NULL for " << type << endl;
}


map<string,void(PGObserver::*)(const LoggerResults*)> PGObserver::op_map{
		make_pair("logger.results.surface.energy",&PGObserver::notifier_surface_energy),
		make_pair("logger.results.volume.energy",&PGObserver::notifier_volume_energy),
		make_pair("logger.results.events",&PGObserver::notifier_events)
	};

void PGObserver::notifier_default(const LoggerResults* lr)
{
	cout << "  Hello from the default notifier for type " << lr->getTypeString() << endl;
}

void PGObserver::notifier_events(const LoggerResults* lr)
{
//	cout << "  It's an events counter with " << dynamic_cast<const EventCount*>(lr)->Nlaunch << endl;
	const EventCount& ec = dynamic_cast<const EventCount&>(*lr);

	dbconn->execParams("UPDATE runresults SET steps=$2,tir=$3,scatter=$4,absorb=$5,fresnel=$6,exit=$7, " \
				"roulettewin=$8,refract=$9,die=$10 WHERE runid=$1;",
				boost::tuples::make_tuple(IDrun,ec.Nabsorb,ec.Ntir,ec.Nscatter,ec.Nabsorb,ec.Nfresnel,ec.Nexit,ec.Nwin,ec.Nrefr,ec.Ndie));

	dbconn->execParams("UPDATE runresults SET intersections=$2,launch=$3,nohit=$4,abnormal=$5,timegate=$6 WHERE runid=$1;",
					boost::tuples::make_tuple(IDrun,ec.Nbound,ec.Nlaunch,ec.Nnohit,ec.Nabnormal,ec.Ntime));
}

void PGObserver::notifier_volume_energy(const LoggerResults* lr)
{
	cout << "  It's a volume energy map" << endl;

	const VolumeArray<double> resultData = dynamic_cast<const VolumeArray<double>&>(*lr);


	//TODO: Below is a hack; code should be hoisted out and either templated or subsumed into boost::serialize
	stringstream ss;

	unsigned nnz=0,i=0;
	double t,sum=0.0;
	for(VolumeArray<double>::const_iterator it=resultData.begin(); it != resultData.end(); ++it,++i)
		if ((t=*it) != 0.0)
		{
			++nnz;
			sum += t;
			ss.write((const char*)&i,4);
			ss.write((const char*)&t,8);
		}

	cout << "Volume array: " << nnz << " nonzeros; size(ss)=" << ss.str().size() << " total=" << sum << endl;

	Blob b(12*nnz,ss.str().c_str());
    Oid oid = dbconn->createLargeObject(b);

    dbconn->execParams("INSERT INTO resultdata(runid,datatype,data_oid,total,bytesize) VALUES ($1,$2,$3,$4,$5)",
        boost::tuples::make_tuple(IDrun,(unsigned)PGDataVolumeEnergy,oid,resultData.getTotal(),b.getSize()));
}

void PGObserver::notifier_surface_energy(const LoggerResults* lr)
{
	cout << "  It's a surface energy map" << endl;

	const SurfaceArray<double> resultData = dynamic_cast<const SurfaceArray<double>&>(*lr);

	//TODO: Below is a hack; code should be hoisted out and either templated or subsumed into boost::serialize
	stringstream ss;

	unsigned nnz=0,i=0;
	double t,sum=0.0;
	for(VolumeArray<double>::const_iterator it=resultData.begin(); it != resultData.end(); ++it,++i)
		if ((t=*it) != 0.0)
		{
			++nnz;
			sum += t;
			ss.write((const char*)&i,4);
			ss.write((const char*)&t,8);
		}

	cout << "Volume array: " << nnz << " nonzeros; size(ss)=" << ss.str().size() << " total=" << sum << endl;

	Blob b(12*nnz,ss.str().c_str());
	Oid oid = dbconn->createLargeObject(b);

    dbconn->execParams("INSERT INTO resultdata(runid,datatype,data_oid,total,bytesize) VALUES ($1,$2,$3,$4,$5)",
        boost::tuples::make_tuple(IDrun,(unsigned)PGDataSurfaceEnergy,oid,resultData.getTotal(),b.getSize()));
}
