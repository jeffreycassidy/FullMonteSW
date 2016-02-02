/*
 * MCKernelBase.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_MCKERNELBASE_HPP_
#define KERNELS_MCKERNELBASE_HPP_

#include "Kernel.hpp"

class MCKernelBase : public Kernel
{
public:
	void 				packetCount(unsigned long long N)			{ Npkt_=N; 						}
	unsigned long long	packetCount()						const	{ return Npkt_; 				}

	void				roulettePrWin(float pr)						{ prwin_=pr;					}
	float				roulettePrWin()						const	{ return prwin_;				}

	void				rouletteWMin(float wmin)					{ wmin_=wmin;					}
	float				rouletteWMin()						const	{ return wmin_;					}

	void				maxSteps(unsigned nmax)						{ Nstep_max_=nmax;				}
	unsigned			maxSteps()							const	{ return Nstep_max_;			}

	void				maxHits(unsigned nmax)						{ Nhit_max_=nmax;				}
	unsigned			maxHits()							const	{ return Nhit_max_;				}

	void 				randSeed(unsigned seed)						{ rngSeed_=seed;				}
	unsigned			randSeed()							const	{ return rngSeed_;				}

	/// Final override for MC: progress is defined in terms of packets simulated
	virtual float				progressFraction()					const final override
			{ return float(simulatedPacketCount())/float(Npkt_); }

	// remaining to be implemented by derived classes
	virtual bool 				done() 					const=0;
	virtual void 				awaitFinish()			=0;
	virtual unsigned long long 	simulatedPacketCount() 	const=0;

protected:
	// remaining to be implemented by derived classes
	virtual void prepare_()=0;
	virtual void start_()=0;

	unsigned long long Npkt_=1000000;
	unsigned Nstep_max_=10000;
	unsigned Nhit_max_=10000;
	float wmin_=1e-4;
	float prwin_=0.1;

	unsigned rngSeed_=1;
};

#endif /* KERNELS_MCKERNELBASE_HPP_ */
