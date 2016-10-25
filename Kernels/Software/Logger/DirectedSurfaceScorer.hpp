/*
 * InternalSurfaceScorer.hpp
 *
 *  Created on: Sep 28, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_LOGGER_DIRECTEDSURFACESCORER_HPP_
#define KERNELS_SOFTWARE_LOGGER_DIRECTEDSURFACESCORER_HPP_

#include "AbstractScorer.hpp"
#include "BaseLogger.hpp"
#include "AtomicMultiThreadAccumulator.hpp"

#include <FullMonteSW/Geometry/Tetra.hpp>
#include <FullMonteSW/Kernels/Software/Packet.hpp>

class DirectedSurfaceScorer : public AbstractScorer
{
public:
	DirectedSurfaceScorer();
	~DirectedSurfaceScorer();

	typedef AtomicMultiThreadAccumulator<double,float> Accumulator;

	virtual void clear() override;
	virtual std::list<OutputData*> results() const override;

	void dim(std::size_t N);


	class Logger : public BaseLogger
	{
	public:
		Logger(Accumulator::ThreadHandle h) : m_handle(h){}
		inline void eventNewTetra(AbstractScorer& S,const Packet& pkt,const Tetra& tet,const unsigned idx)
		{
			auto& DS = static_cast<DirectedSurfaceScorer&>(S);

			int IDf=tet.IDfs[idx];
			if ((tet.faceFlags >> (idx << 3)) & 0x1)		// check if LSB of faceFlags is set for this face
				m_handle.accumulate(DS.m_accumulator,IDf+DS.m_elements,pkt.weight());
		}

		void commit(AbstractScorer& S)
		{}
	private:
		Accumulator::ThreadHandle m_handle;
	};


	Logger createLogger();
	Logger get_logger(){ return createLogger(); }

private:
	unsigned 	m_elements=0;
	Accumulator m_accumulator;
};



#endif /* KERNELS_SOFTWARE_LOGGER_DIRECTEDSURFACESCORER_HPP_ */
