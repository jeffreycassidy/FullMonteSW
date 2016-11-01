/*
 * CylAbsorption.hpp
 *
 *  Created on: Oct 30, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_CYLABSORPTION_HPP_
#define KERNELS_SOFTWARE_LOGGER_CYLABSORPTION_HPP_

#include "AbstractScorer.hpp"

#include "BaseLogger.hpp"

#include "AtomicMultiThreadAccumulator.hpp"

/** Logs absorption events per volume element, using a cylindrical coordinate system.
 *
 */

class CylAbsorptionScorer : public AbstractScorer
{
public:
	typedef AtomicMultiThreadAccumulator<double,float> Accumulator;

	class Logger: public BaseLogger
	{
	public:
		typedef CylAbsorptionScorer Scorer;

		Logger(Logger&&) = default;

		inline void eventAbsorb(AbstractScorer& S,Point3 p,unsigned IDt,double w0,double dw)
			{
				float r = std::sqrt(p[0]*p[0] + p[1]*p[1]);
				float z = p[2];

				unsigned ri = std::floor(r/m_dr);
				unsigned zi = std::floor(z/m_dz);
				if ((ri < m_Nr) & (zi < m_Nz))
					m_handle.accumulate(static_cast<CylAbsorptionScorer&>(S).m_acc,ri+m_Nr*zi,dw);
				else
					m_wOutOfBounds += dw;
			}

		void clear();
		void commit(AbstractScorer& S);

	private:
		typename Accumulator::ThreadHandle m_handle;
	};

	CylAbsorptionScorer();
	virtual ~CylAbsorptionScorer();

	/// Number of bins in r and z directions
	void dim(unsigned Nr,unsigned Nz);
	std::array<float,2> dim() const { return std::array<unsigned,2>{ m_Nr,m_Nz}; }

	/// Set extent of scoring area (preserves number of bins, changes resolution to match)
	void extent(float r,float z);

	/// Set scoring spatial resolution (preserves number of bins, extent will change)
	void resolution(float dr,float dz);

	virtual std::list<OutputData*> results() const override;
	virtual void clear() override;

	Logger get_logger();

	const Accumulator& accumulator() const { return m_acc; }

private:
	float						m_dr=0.01f;
	float						m_dz=0.01f;
	unsigned					m_Nr=100;
	unsigned					m_Nz=100;
	double						m_wOutOfBounds=0.0;
	AtomicMultiThreadAccumulator<double,float> m_acc;
};

#endif /* KERNELS_SOFTWARE_LOGGER_CYLABSORPTION_HPP_ */
