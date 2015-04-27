/*
 * Kernel.cpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#include "Kernel.hpp"

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <FullMonte/Kernels/Software/mainloop.cpp>

#include <boost/random/additive_combine.hpp>

#include <memory>

using namespace std;

void Kernel::clearSources()
{
	for(SourceDescription* s : src_)
		delete(s);
	src_.clear();
}

void Kernel::setSources(const vector<SourceDescription*>& S)
{
	clearSources();
	src_.resize(S.size());
	boost::copy(S | boost::adaptors::transformed([](SourceDescription* S){ return S->clone(); }),
			src_.begin());
}

void Kernel::setSource(SourceDescription* s)
{
	src_.resize(1);
	src_[0] = s->clone();
}


void MonteCarloKernelBase::setMaterials(const vector<SimpleMaterial>& mats)
{
	mat_.resize(mats.size());
	boost::copy(
		mats | boost::adaptors::transformed([](SimpleMaterial sm){ return Material(sm.mu_a,sm.mu_s,sm.g,sm.n); }),
		mat_.begin());
}


template<class RNG>void TetraMCKernel<RNG>::prepare_()
{
	if (src_.size()==0)
	{
		cerr << "ERROR: No sources specified" << endl;
		throw std::logic_error("ERROR: No sources specified");
	}

	// copy sources
	cout << "Sources: " << endl;
	for(const SourceDescription* s : src_)
		cout << *s << endl;

	emitter_.reset(SourceEmitterFactory<RNG_SFMT_AVX>(M_,src_));

	// copy materials

	cout << "Materials: " << endl;
	for(const Material& m : mat_)
		cout << m << endl;

	vector<unsigned> hist;
	for(unsigned i=0; i<=M_.getNt(); ++i)
	{
		unsigned mat = M_.getMaterial(i);
		if (mat >= hist.size())
			hist.resize(mat+1);
		hist[mat]++;
	}

	cout << "Material | Tetra Count" << endl;
	unsigned i=0;
	for(auto h : hist)
		cout << ++i << ": " << h << endl;
}

void TetraSurfaceKernel::prepare_()
{
	// display debug output
	TetraMCKernel::prepare_();
}

//void TetraVolumeKernel::start()
//{
//	// Set up logger
//    logger = make_tuple(
//    		LoggerEventMT(),
//    		LoggerConservationMT(),
//    		LoggerVolume<QueuedAccumulatorMT<double>>(K_.mesh,1<<10)
//    		);
//
//    // Run it
//}

void TetraSurfaceKernel::start_()
{
	logger_.reset(new LoggerType(make_tuple(
			LoggerEventMT(),
			LoggerConservationMT(),
			LoggerSurface<QueuedAccumulatorMT<double>>(M_,1<<10)
	)));

	workers_.resize(Nth_,nullptr);

	boost::random::ecuyer1988 seeds_generator(rngSeed_);

	seeds_generator.discard(10000);

	for(unsigned i=0;i<Nth_;++i)
	{
		workers_[i] = new TetraMCKernelThread<LoggerWorker,RNG_SFMT_AVX>(*this,get_worker(*logger_),seeds_generator(),Npkt_/Nth_);
		seeds_generator.discard(100);
	}

	cout << "Starting " << Nth_ << " threads" << endl;
	boost::for_each(workers_, [](SimMCThread* w){ assert(w); w->startAsync(); });
}

void TetraSurfaceKernel::finish_()
{
	auto res = std::make_tuple(
			get<0>(*logger_).getResults(),
			get<1>(*logger_).getResults(),
			get<2>(*logger_).getResults());

	results_.clear();

	// clone because storage above is of automatic duration
	tuple_for_each(res, [this] (const LoggerResults& r) { this->results_.push_back(std::shared_ptr<const LoggerResults>(r.clone())); });

	cout << "Results are available" << endl;

	for(auto p : results_)
		p->summarize(cout);

	cout << endl << endl << "Result types available: ";
	for(auto p : results_)
		cout << " " << p->getTypeString();
	cout << endl;

	cout << "Kernel is finished" << endl;
}
