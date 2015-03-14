#include "Notifier.hpp"
#include "LoggerEvent.hpp"
#include "LoggerConservation.hpp"

#include "LoggerVolume.hpp"
#include "LoggerSurface.hpp"

#include "Observer.hpp"

void OStreamObserver::notify_start(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts,unsigned IDc)
{
	os << "Run started; problem definition:" << endl << geom << cfg << endl << opts << endl;
}

void OStreamObserver::notify_finish(boost::timer::cpu_times t)
{
	os << "Run finished, elapsed time: " << format(t) << endl;
}

void OStreamObserver::notify_result(const LoggerResults& lr)
{
	lr.summarize(cout);
}
