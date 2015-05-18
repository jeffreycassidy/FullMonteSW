#ifndef MAINLOOP_HPP_INCLUDED_
#define MAINLOOP_HPP_INCLUDED_


#include <FullMonte/Kernels/Kernel.hpp>
#include <boost/timer/timer.hpp>

#include <boost/random/additive_combine.hpp>

#include <FullMonte/Kernels/Software/FullMonte.hpp>
#include <FullMonte/Geometry/Material.hpp>
#include <FullMonte/Geometry/Tetra.hpp>

#include <mutex>
#include <thread>

#include <FullMonte/oldstuff/TupleStuff.hpp>

#include "SourceEmitter.hpp"
#include "LineSourceEmitter.hpp"

/** Represents a thread-safe simulation worker.
 * Reasonably lightweight, copies only the configuration (options & geometry are referenced).
 *
 */

template<class Logger,class RNG> class TetraMCKernelThread : public SimMCThreadBase {
	const TetraMCKernel<RNG>& K_;

	std::thread t;

	Logger logger;								///< Logger object

	// Loop counters
	unsigned long long N_=0;						///< Total number of items to do
	unsigned long long i_=0;						///< Number of items completed

    RNG rng;

    static const unsigned HG_BUFFERS=20;	///< Number of distinct g values to buffer
    static const unsigned HG_BUFSIZE=8;		///< Number of spin vectors to buffer

    float * hg_buffers[HG_BUFFERS];			// should be float * const but GCC whines about not being initialized
    unsigned hg_next[HG_BUFFERS];


    /// Main body which does all the work
    void thread_loop()
    {
    	assert(K_.emitter_);

    	cout << "Running thread loop with " << N_ << " packets" << endl;
    	pair<Packet,unsigned> tmp;
    	Packet& pkt=tmp.first;
    	unsigned& IDt=tmp.second;

    	for(;i_<N_; ++i_)
    	{
    		tmp = K_.emitter_->emit(rng);
    		doOnePacket(pkt,IDt);
    	}

    	log_event(logger,Events::commit);
    }

    int doOnePacket(Packet pkt,unsigned IDt);

    void awaitFinish() { t.join(); }


public:

    typedef uint32_t Seed;

    // move-constructs the logger and gets thread ready to run but does not actually start it (call start())
	TetraMCKernelThread(const TetraMCKernel<RNG>& K,Logger&& logger_,const Seed& seed_,unsigned long long N) :
		K_(K),
		logger(std::move(logger_)),
		N_(N),
		rng(4096,seed_)
	{
		// Allocate HG buffers (currently necessary to get correct alignment; future GCC may relieve the requirement)
		//
		void *p;
		posix_memalign(&p,32,HG_BUFFERS*HG_BUFSIZE*4*sizeof(float));
		for(unsigned i=0; i<HG_BUFFERS; ++i)
		{
			hg_buffers[i] = (float*)p + 4*HG_BUFSIZE*i;
			hg_next[i] = HG_BUFSIZE;
		}
	}


    /// Temporary kludge to handle HG function generation
    inline __m128 getNextHG(unsigned matID){
    	if (hg_next[matID] == HG_BUFSIZE){
    		K_.mat_[matID].VectorHG(rng.draw_m256f8_pm1(),rng.draw_m256f8_uvect2(),hg_buffers[matID]);
    		hg_next[matID]=0;
    	}
    	return _mm_load_ps(hg_buffers[matID] + ((hg_next[matID]++)<<2));
    }

	void startAsync()	{
		cout << "Starting a thread" << endl;
		t=std::thread(mem_fn(&TetraMCKernelThread<Logger,RNG>::thread_loop),this);
		cout << "Returning from startAsync" << endl;
	}
	void finishAsync()	{ t.join(); }

	~TetraMCKernelThread(){ free(hg_buffers[0]); }

	bool done() const { return i_==N_; }
	unsigned long long getSimulatedPacketCount() const { return i_; }
	std::pair<unsigned long long,unsigned long long> getProgress() const { return make_pair(i_,N_); }
};

#endif
