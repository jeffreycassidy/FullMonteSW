#include "OStreamObserver.hpp"
#include "Logger.hpp"

void OStreamObserver::notify_create(const SimGeometry& geom,const RunConfig& cfg,const RunOptions& opts)
{
	os << "Run created; problem definition:" << endl << geom << cfg << endl << opts << endl;
}

void OStreamObserver::notify_start()
{
	os << "Run started" << endl;
}

void OStreamObserver::notify_finish(boost::timer::cpu_times t)
{
	os << "Run finished, elapsed time: " << format(t) << endl;
}

void OStreamObserver::notify_result(const LoggerResults& lr)
{
	lr.summarize(cout);
}
