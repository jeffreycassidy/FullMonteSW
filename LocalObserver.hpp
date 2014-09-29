#pragma once
#include "Observer.hpp"
#include "LoggerVolume.hpp"
#include <vector>
#include <string>

class LocalObserver {
	std::string prefix;
	const TetraMesh& M;
	const std::vector<Material>& mats;
public:
	LocalObserver(const TetraMesh& M_,const std::vector<Material>& mats_,std::string prefix_) :
		prefix(prefix_),M(M_),mats(mats_){};
	~LocalObserver(){};

	virtual void notify_create(const SimGeometry&,const RunConfig& cfg,const RunOptions& opts);
	virtual void notify_start();
	virtual void notify_finish(boost::timer::cpu_times t);
	virtual void notify_result(const LoggerResults& r);
};
