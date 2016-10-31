/*
 * VolumeAbsorptionScorer.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_VOLUMEABSORPTIONSCORER_HPP_
#define KERNELS_SOFTWARE_LOGGER_VOLUMEABSORPTIONSCORER_HPP_

#include "AbstractScorer.hpp"

#include "QueuedMultiThreadAccumulator.hpp"
#include "BaseLogger.hpp"

/** Logs absorption events per volume element
 *
 */

class VolumeAbsorptionScorer : public AbstractScorer
{
public:
	typedef QueuedMultiThreadAccumulator<double,float> Accumulator;

	class Logger: public BaseLogger
	{
	public:
		typedef VolumeAbsorptionScorer Scorer;

		explicit Logger(std::size_t qSize);
		Logger(Logger&&) = default;

		inline void eventAbsorb(AbstractScorer& S,Point3 p,unsigned IDt,double w0,double dw)
			{ m_handle.accumulate(static_cast<VolumeAbsorptionScorer&>(S).m_acc,IDt,dw); }

		void clear();
		void commit(AbstractScorer& S);

	private:
		typename Accumulator::ThreadHandle m_handle;
	};

	VolumeAbsorptionScorer();
	~VolumeAbsorptionScorer();

	void dim(unsigned N);
	std::size_t dim() const { return m_acc.size(); }

	void queueSize(unsigned q);

	virtual std::list<OutputData*> results() const override;
	virtual void clear() override;

	Logger get_logger();

private:
	unsigned m_queueSize=1024;

	QueuedMultiThreadAccumulator<double,float> m_acc;
};

#endif /* KERNELS_SOFTWARE_LOGGER_VOLUMEABSORPTIONSCORER_HPP_ */
