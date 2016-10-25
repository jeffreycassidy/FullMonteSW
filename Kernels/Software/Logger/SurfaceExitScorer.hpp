/*
 * VolumeAbsorptionScorer.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_SurfaceExitScorer_HPP_
#define KERNELS_SOFTWARE_LOGGER_SurfaceExitScorer_HPP_

#include "AbstractScorer.hpp"
#include "AtomicMultiThreadAccumulator.hpp"
#include <vector>
#include "BaseLogger.hpp"

/**
 * Since surface exit is a relatively rare phenomenon, use an atomic accumulator to track.
 */

class SurfaceExitScorer : public AbstractScorer
{
public:
	class Logger;

	typedef AtomicMultiThreadAccumulator<double,float> Accumulator;

	SurfaceExitScorer();
	~SurfaceExitScorer();

	void dim(std::size_t N);

	double total() const;

	virtual std::list<OutputData*> results() const override;
	virtual void clear() override;

	Logger get_logger();

	const Accumulator& accumulator() const;

private:
	/// Number of surface elements. Used to offset the IDf's [-Nf,Nf] to get positive values [0,2Nf+1).
	unsigned m_elements=0;

	/// The accumulator
	Accumulator		m_acc;
};


class SurfaceExitScorer::Logger: public BaseLogger
{
public:
	typedef SurfaceExitScorer Scorer;

	Logger();
	~Logger();

	Logger(Logger&& lv_) = default;
	Logger(const Logger& lv_) = delete;

	inline void eventExit(AbstractScorer& S,const Ray3,int IDf,double w);

	void clear();
	void commit(AbstractScorer& S);

private:
	Accumulator::ThreadHandle		m_handle;
};

inline const SurfaceExitScorer::Accumulator& SurfaceExitScorer::accumulator() const
{
	return m_acc;
}

inline void SurfaceExitScorer::Logger::clear()
{
	m_handle.clear();
}

inline void SurfaceExitScorer::Logger::commit(AbstractScorer& S)
{
	SurfaceExitScorer& SS = static_cast<SurfaceExitScorer&>(S);
	m_handle.commit(SS.m_acc);
}

inline void SurfaceExitScorer::Logger::eventExit(AbstractScorer& S,const Ray3,int IDf,double w)
{
	SurfaceExitScorer& SS = static_cast<SurfaceExitScorer&>(S);
	m_handle.accumulate(SS.m_acc,IDf+SS.m_elements,float(w));
}

#endif /* KERNELS_SOFTWARE_LOGGER_SurfaceExitScorer_HPP_ */
