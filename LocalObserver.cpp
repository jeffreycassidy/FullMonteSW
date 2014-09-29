#include "LocalObserver.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include "fm-postgres/fmdbexportcase.hpp"
#include "Material.hpp"
#include <algorithm>

using namespace std;

void LocalObserver::notify_create(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts)
{
	cout << "local observer: created" << endl;
};

void LocalObserver::notify_start()
{
	cout << "local observer: started" << endl;
};

void LocalObserver::notify_finish(boost::timer::cpu_times t)
{
	cout << "local observer: finished after " << t.wall*1e-9 << " seconds" << endl;
}

void LocalObserver::notify_result(const LoggerResults& r)
{
	stringstream ss;

	string type=r.getTypeString();

	if (type == "logger.results.volume.energy")
	{
		const VolumeArray<double>& E_v = dynamic_cast<VolumeArray<double>&>(r);

		vector<double> E_vv;
		std::copy(E_v.begin(),E_v.end(),std::back_inserter(E_vv));

		vector<double> phi_v = volumeEnergyToFluence(E_vv,M,mats);

		ss << prefix << ".phi_v.txt";
		ofstream os(ss.str().c_str());

		for(double phi : phi_v)
			os << phi << endl;
		os.close();
	}
	else {
		cout << "local observer: data type '" << type << "' not handled yet" << endl;
	}
}
