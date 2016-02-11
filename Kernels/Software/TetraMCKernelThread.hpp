#ifndef MAINLOOP_HPP_INCLUDED_
#define MAINLOOP_HPP_INCLUDED_

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>
#include <FullMonte/Kernels/Software/Emitters/Base.hpp>
#include <FullMonte/Kernels/Software/Logger/Logger.hpp>

#include <FullMonte/Geometry/Tetra.hpp>

#include <FullMonte/Kernels/Software/TetraMCKernel.hpp>
#include <FullMonte/Kernels/Software/Material.hpp>

#include "SSEMath.hpp"

template<class RNG>template<class Logger> class TetraMCKernel<RNG>::Thread : public ThreadedMCKernelBase::Thread
{
public:
	~Thread(){ free(hg_buffers[0]); }

    // move-constructs the logger and gets thread ready to run but does not actually start it (call doWork())
	Thread(const TetraMCKernel<RNG>& K,Logger&& logger_);

    /// Main body which does all the work
    void doWork();
    int doOnePacket(LaunchPacket pkt);
    inline __m128 getNextHG(unsigned matID);

    /// Seed the RNG
    void seed(unsigned s)
    {
    	rng.seed(s);
    }

private:
	const TetraMCKernel<RNG>& 		m_parentKernel;
	Logger 	logger;								///< Logger object
    RNG 	rng;

    static const unsigned HG_BUFFERS=20;		///< Number of distinct g values to buffer
    static const unsigned HG_BUFSIZE=8;			///< Number of spin vectors to buffer

    float * hg_buffers[HG_BUFFERS];				// should be float * const but GCC whines about not being initialized
    unsigned hg_next[HG_BUFFERS];
};


template<class RNG>template<class Logger>inline __m128 TetraMCKernel<RNG>::Thread<Logger>::getNextHG(unsigned matID)
{
	if (hg_next[matID] == HG_BUFSIZE){
		m_parentKernel.mat_[matID].VectorHG(rng.draw_m256f8_pm1(),rng.draw_m256f8_uvect2(),hg_buffers[matID]);
		hg_next[matID]=0;
	}
	return _mm_load_ps(hg_buffers[matID] + ((hg_next[matID]++)<<2));
}


// move-constructs the logger and gets thread ready to run but does not actually start it (call start())
template<class RNG>template<class Logger>TetraMCKernel<RNG>::Thread<Logger>::Thread(const TetraMCKernel<RNG>& K,Logger&& logger_) :
	m_parentKernel(K),
	logger(std::move(logger_))
{
	// Allocate HG buffers (currently necessary to get correct alignment; future GCC may relieve the requirement)
	//
	void *p;
	int st = posix_memalign(&p,32,HG_BUFFERS*HG_BUFSIZE*4*sizeof(float));

	if (st || !p || ((unsigned long long)(p) & 0x1F))
		throw std::bad_alloc();

	for(unsigned i=0; i<HG_BUFFERS; ++i)
	{
		hg_buffers[i] = (float*)p + 4*HG_BUFSIZE*i;
		hg_next[i] = HG_BUFSIZE;
	}
}

template<class RNG>template<class Logger>void TetraMCKernel<RNG>::Thread<Logger>::doWork()
{
	LaunchPacket lpkt;

	for( ; m_nPktDone < m_nPktReq; ++m_nPktDone)
	{
		lpkt = m_parentKernel.m_emitter->emit(rng);
		doOnePacket(lpkt);
	}

	log_event(logger,Events::commit);
}

enum TerminationResult { Continue=0, RouletteWin, RouletteLose, TimeGate, Other=-1  };

    /**
     * Conducts termination check (including roulette).
     *
     * @param rng Random-number generator
     * @param mat Material
     * @param region Region
     *
     * @returns pair<RouletteResult,double> = < result, dw> where dw is change in packet weight due to roulette
     *
     * @tparam RNG
     */

typedef Tetra Region;

template<class RNG>pair<TerminationResult,double> terminationCheck(const double wmin,const double prwin,RNG& rng,Packet& pkt,const Material& mat,const Region& region)
{
    // do roulette
	double w0=pkt.w;
    if (pkt.w < wmin)
    {
    	if (rng.draw_float_u01() < prwin)
    	{
    		pkt.w /= prwin;
    		return make_pair(RouletteWin,pkt.w-w0);
    	}
    	else
    		return make_pair(RouletteLose,-pkt.w);
    }
    else
    	return make_pair(Continue,0.0);
}


/** would be nice to hoist the Henyey-Greenstein RNG out of the worker into the RNG class (or Material class?) */

template<class Worker>inline bool scatter(Packet& pkt,Worker& wkr,const Material& mat,const Region& region)
{
	if (!mat.isScattering())
		return false;

	__m128 spinmatrix = wkr.getNextHG(region.matID);
	pkt.dir = pkt.dir.scatter(SSE::Vector<4>(spinmatrix));
	return true;
}

inline pair<float,float> absorb(const Packet& pkt,const Material& mat,const Tetra& tet)
{
	float w0=pkt.weight();
	float dw=w0*mat.getAbsorbedFraction();
	return make_pair(w0-dw,dw);
}


template<class RNG>template<class Logger>int TetraMCKernel<RNG>::Thread<Logger>::doOnePacket(LaunchPacket lpkt)
{
    unsigned Nhit,Nstep;
    StepResult stepResult;
    Tetra currTetra = m_parentKernel.m_mesh->getTetra(lpkt.element);
    Material currMat = m_parentKernel.mat_[currTetra.matID];

    unsigned IDt=lpkt.element;

    float f_tmp[4] __attribute__((aligned(16)));
    float &n1 = f_tmp[0];
    float &n2 = f_tmp[1];
    float &ratio = f_tmp[2];
    unsigned IDt_next=IDt, IDm=currTetra.matID, IDm_next=IDm, IDm_bound;

    Packet pkt(lpkt);

    log_event(logger,Events::launch,make_pair(pkt.p,__m128(pkt.direction())),IDt,1.0);

    // start another hop
    for(Nstep=0; Nstep < m_parentKernel.Nstep_max_; ++Nstep)
    {
        // draw a hop length; pkt.s = { physical distance, MFPs to go, time, 0 }
        pkt.s = _mm_mul_ps(rng.draw_m128f1_exp(),currMat.s_init);

        // attempt hop
        stepResult = currTetra.getIntersection(pkt.p,__m128(pkt.direction()),pkt.s);
        pkt.p      = stepResult.Pe;

        // loop while hitting a face in current step
        for(Nhit=0; stepResult.hit && Nhit < m_parentKernel.Nhit_max_; ++Nhit)
        {
            // extremely rarely, this can be a problem; we get no match in the getIntersection routine
            if(stepResult.idx > 3)
            {
                cerr << "Abnormal condition: stepResult.idx=" << stepResult.idx << ", IDte=" << stepResult.IDte << endl;
                cerr << "  Terminating packet" << endl;
                log_event(logger,Events::nohit,pkt,currTetra);
                return -1;
            }
            pkt.s = _mm_add_ps(pkt.s,_mm_mul_ps(stepResult.distance,currMat.s_prop));
            IDm_bound = m_parentKernel.m_mesh->getMaterial(stepResult.IDte);
            if (IDm == IDm_bound) { // no material change
            	log_event(logger,Events::boundary,pkt.p,stepResult.IDfe,IDt,stepResult.IDte);
                IDt_next = stepResult.IDte;
            }
            else // boundary with material change
            {
                n2 = m_parentKernel.mat_[IDm_bound].getn();
                n1 = currMat.getn();

                if (n1 == n2 || m_parentKernel.mat_[IDm_bound].isMatched()) // no refractive index difference
                {
                	log_event(logger,Events::boundary,pkt.p,stepResult.IDfe,IDt,stepResult.IDte);
                    IDt_next = stepResult.IDte;
                }
                else {
                	log_event(logger,Events::interface,make_pair(pkt.p,__m128(pkt.direction())),stepResult.IDfe,stepResult.IDte);
                    __m128 Fn[4];

                    Fn[0] = currTetra.nx;
                    Fn[1] = currTetra.ny;
                    Fn[2] = currTetra.nz;
                    Fn[3] = _mm_setzero_ps();
                    _MM_TRANSPOSE4_PS(Fn[0],Fn[1],Fn[2],Fn[3]);
                    __m128 normal = Fn[stepResult.idx];
                    __m128 costheta = _mm_min_ps(
                    						_mm_set1_ps(1.0),
											_mm_sub_ps(
													_mm_setzero_ps(),
													__m128(dot(SSE::Vector3(normal),pkt.direction()))));

                    ratio = n1/n2;
					__m128 n1_n2_ratio = _mm_load_ps(f_tmp);

                    __m128 sini_cosi_sint_cost = RefractSSE(n1_n2_ratio,costheta);


                	__m128 newdir;

                    if (_mm_movemask_ps(_mm_cmplt_ss(_mm_set_ss(1.0),_mm_movehl_ps(sini_cosi_sint_cost,sini_cosi_sint_cost)))&1)
                    {
                        newdir = reflect(__m128(pkt.direction()),normal,sini_cosi_sint_cost);
                        log_event(logger,Events::reflect,pkt.p,__m128(pkt.direction()));
                    }
                    else {
                    	__m128 d_p = _mm_add_ps(
									__m128(pkt.direction()),
									_mm_mul_ps(
										normal,
										costheta));

					    __m128 pr = FresnelSSE(n1_n2_ratio,sini_cosi_sint_cost);
                        if (_mm_movemask_ps(_mm_cmplt_ss(rng.draw_m128f1_u01(),pr))&1)
                        {
                            newdir = reflect(__m128(pkt.direction()),normal,sini_cosi_sint_cost);
                            log_event(logger,Events::fresnel,pkt.p,__m128(pkt.direction()));
                        }
                        else {
						    newdir = _mm_sub_ps(
							    _mm_mul_ps(
								    d_p,
								    _mm_shuffle_ps(n1_n2_ratio,n1_n2_ratio,_MM_SHUFFLE(2,2,2,2))),
							    _mm_mul_ps(
								    normal,
								    _mm_shuffle_ps(sini_cosi_sint_cost,sini_cosi_sint_cost,_MM_SHUFFLE(3,3,3,3))));
						    log_event(logger,Events::refract,pkt.p,__m128(pkt.direction()));
                            IDt_next = stepResult.IDte;
                        // configure material properties
                        } // if: fresnel reflection
                    }
                    pkt.dir = PacketDirection(SSE::UnitVector3(SSE::Vector3(newdir),SSE::NoCheck));
                } // if: refractive index difference
            } // if: material change

            if (IDt_next == 0)
            {
            	log_event(logger,Events::exit,make_pair(pkt.p,__m128(pkt.direction())),stepResult.IDfe,pkt.w);
                return 0;
            }
            else if (IDt != IDt_next)
            {
                IDt = IDt_next;
                IDm_next = IDm_bound;
                currTetra = m_parentKernel.m_mesh->getTetra(IDt);
            }

            if (IDm != IDm_next)
            {
                IDm = IDm_next;
                currMat = m_parentKernel.mat_[IDm];
                pkt.s = _mm_div_ss(_mm_movehdup_ps(pkt.s), _mm_set_ss(currMat.getMuT()));
            }
            stepResult=currTetra.getIntersection(pkt.p,__m128(pkt.direction()),pkt.s);
            pkt.p   = stepResult.Pe;
        }
        if (Nhit >= m_parentKernel.Nhit_max_)
        {
        	cerr << "Terminated due to unusual number of interface hits" << endl;
        	log_event(logger,Events::abnormal,pkt,Nstep,Nhit);
        	return -2;
        }

        // Absorption process
        double dw,w0=pkt.w;
        tie(pkt.w,dw) = absorb(pkt,currMat,currTetra);

        if (dw != 0.0)
        	log_event(logger,Events::absorb,pkt.p,IDt,pkt.w,dw);

        // Termination logic
        TerminationResult term;
        w0=pkt.w;

        tie(term,dw)=terminationCheck(m_parentKernel.wmin_,m_parentKernel.prwin_,rng,pkt,currMat,currTetra);

        switch(term){
        case Continue:								// Continues, no roulette
        	break;

        case RouletteWin:							// Wins at roulette, dw > 0
        	log_event(logger,Events::roulettewin,w0,pkt.w);
        	break;

        case RouletteLose:							// Loses at roulette, dw < 0
        	log_event(logger,Events::roulettedie,pkt.w);
        	return 0;
        	break;

        case TimeGate:								// Expires due to time gate
        	log_event(logger,Events::timegate,pkt);
        	return 1;
        	break;

        case Other:
        default:
        	break;
        }

    	if (scatter(pkt,*this,currMat,currTetra))
    		log_event(logger,Events::scatter,__m128(pkt.direction()),__m128(pkt.direction()),currMat.getParam_g());
    }

    // should only fall through to here in abnormal circumstances (too many steps)
//    cerr << "Abnormal condition: packet retired after " << Nstep << " steps" << endl;
//    cerr << "p=" << pkt.p << " d=" << __m128(pkt.direction()) << " a=" << pkt.a << " b=" << pkt.b << endl;
//    cerr << "IDt=" << IDt << " |d|=" << norm(__m128(pkt.direction())) << endl;

    log_event(logger,Events::abnormal,pkt,Nstep,Nhit);
    return -1;
}


#endif
