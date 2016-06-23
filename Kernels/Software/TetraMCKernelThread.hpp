#ifndef MAINLOOP_HPP_INCLUDED_
#define MAINLOOP_HPP_INCLUDED_

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <iomanip>
#include <FullMonteSW/Kernels/Software/Emitters/Base.hpp>
#include <FullMonteSW/Kernels/Software/Logger/Logger.hpp>

#include <FullMonteSW/Geometry/Tetra.hpp>

#include <FullMonteSW/Kernels/Software/TetraMCKernel.hpp>

#include "BlockRandomDistribution.hpp"
#include "HenyeyGreenstein.hpp"
#include "FloatUnitExpDistribution.hpp"
#include "FloatU01Distribution.hpp"

#include "SSEConvert.hpp"	// for as_array

#define MAX_MATERIALS 32

class AutoStreamBuffer;
template<class T>AutoStreamBuffer& operator<<(AutoStreamBuffer& sb,const T& v);

class AutoStreamBuffer
{
public:
	AutoStreamBuffer(std::ostream& os) : m_os(os){}
	~AutoStreamBuffer()
	{
		m_os << m_ss.str();
	}

private:
	std::ostream&		m_os;
	std::stringstream	m_ss;

	template<class T>friend AutoStreamBuffer& operator<<(AutoStreamBuffer&,const T&);
};

template<class T>AutoStreamBuffer& operator<<(AutoStreamBuffer& sb,const T& v)
{
	sb.m_ss << v;
	return sb;
}


template<class RNGType>template<class Logger> class alignas(32) TetraMCKernel<RNGType>::Thread : public ThreadedMCKernelBase::Thread
{
public:
	typedef RNGType RNG;

	~Thread(){ }

    // move-constructs the logger and gets thread ready to run but does not actually start it (call doWork())
	Thread(const TetraMCKernel<RNGType>& K,Logger&& logger_);

    /// Main body which does all the work
    void doWork();
    int doOnePacket(LaunchPacket pkt);

    /// Seed the RNG
    void seed(unsigned s)
    {
    	m_rng.seed(s);
    }

    inline bool scatter(Packet& pkt,const TetraKernelBase::Material& mat,const Tetra& region);

private:
	const TetraMCKernel<RNG>& 		m_parentKernel;
	Logger 	logger;											///< Logger object
    RNG 	m_rng;											///< Random-number generator

};


// move-constructs the logger and gets thread ready to run but does not actually start it (call start())
template<class RNG>template<class Logger>TetraMCKernel<RNG>::Thread<Logger>::Thread(const TetraMCKernel<RNG>& K,Logger&& logger_) :
	m_parentKernel(K),
	logger(std::move(logger_))
{
	if (m_parentKernel.materials().size() > MAX_MATERIALS)
		throw std::logic_error("More materials than the max allowed");

	for(unsigned i=0;i<m_parentKernel.materials().size(); ++i)
		m_rng.gParamSet(i,m_parentKernel.materials()[i].g());
}

template<class RNG>template<class Logger>void TetraMCKernel<RNG>::Thread<Logger>::doWork()
{
	LaunchPacket lpkt;

	for( ; m_nPktDone < m_nPktReq; ++m_nPktDone)
	{
		lpkt = m_parentKernel.m_emitter->emit(m_rng);
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

template<class RNG>pair<TerminationResult,double> terminationCheck(const double wmin,const double prwin,RNG& rng,Packet& pkt,const TetraKernelBase::Material& mat,const Region& region)
{
    // do roulette
	double w0=pkt.w;
    if (pkt.w < wmin)
    {
    	if (*rng.floatU01() < prwin)
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

template<class RNGType>template<class Logger>inline bool TetraMCKernel<RNGType>::Thread<Logger>::scatter(Packet& pkt,const TetraKernelBase::Material& mat,const Region& region)
{
	if (!mat.scatters())
		return false;

	__m128 spinmatrix = _mm_load_ps(m_rng.hg(region.matID));

//#ifndef NDEBUG
//	float f[4];
//	_mm_store_ps(f,spinmatrix);
//	std::cout << std::fixed << std::setprecision(6) << std::setw(9) << f[0] << ' ' << std::setw(9) << f[1] << ' ' << std::setw(9) << f[2] << ' ' << std::setw(9) <<  f[3] << std::endl;
//#endif

	pkt.dir = pkt.dir.scatter(SSE::Vector<4>(spinmatrix));
	return true;
}

inline pair<float,float> absorb(const Packet& pkt,const TetraKernelBase::Material& mat,const Tetra& tet)
{
	float w0=pkt.weight();
	float dw=w0*mat.absorbedFraction();
	return make_pair(w0-dw,dw);
}


template<class RNG>template<class Logger>int TetraMCKernel<RNG>::Thread<Logger>::doOnePacket(LaunchPacket lpkt)
{
    unsigned Nhit,Nstep;
    StepResult stepResult;
    Tetra currTetra = m_parentKernel.m_mesh->getTetra(lpkt.element);
    Material currMat = m_parentKernel.m_mats[currTetra.matID];

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
        assert(pkt.dir.d.check(SSE::Silent,1e-4) && pkt.dir.a.check(SSE::Silent,1e-4) && pkt.dir.b.check(SSE::Silent,1e-4));

        // draw a hop length; pkt.s = { physical distance, MFPs to go, time, 0 }
        pkt.s = _mm_mul_ps(
        			_mm_load1_ps(m_rng.floatExp()),
					currMat.m_init);

    	assert(0 <= _mm_cvtss_f32(pkt.s));
    	assert(_mm_cvtss_f32(pkt.s) < 1e3);

        // attempt hop
        stepResult = currTetra.getIntersection(pkt.p,__m128(pkt.direction()),pkt.s);
        pkt.p      = stepResult.Pe;

        // loop while hitting a face in current step
        for(Nhit=0; stepResult.hit && Nhit < m_parentKernel.Nhit_max_; ++Nhit)
        {
            // extremely rarely, this can be a problem; we get no match in the getIntersection routine
            if(stepResult.idx > 3)
            {
            	{
            		AutoStreamBuffer b(cerr);
            		b << "Abnormal condition: stepResult.idx=" << stepResult.idx << ", IDte=" << stepResult.IDte <<
            			"  Terminating packet\n";
            	}
                log_event(logger,Events::nohit,pkt,currTetra);
                return -1;
            }
            pkt.s = _mm_add_ps(
            			pkt.s,
						_mm_mul_ps(
								stepResult.distance,
								currMat.m_prop));

            IDm_bound = m_parentKernel.m_mesh->getMaterial(stepResult.IDte);

            if (IDm == IDm_bound) { // no material change
            	log_event(logger,Events::boundary,pkt.p,stepResult.IDfe,IDt,stepResult.IDte);
                IDt_next = stepResult.IDte;
            }
            else // boundary with material change
            {
                n2 = m_parentKernel.m_mats[IDm_bound].n();
                n1 = currMat.n();

                if (n1 == n2) // no refractive index difference
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
                        if (_mm_movemask_ps(
                        		_mm_cmplt_ss(
                        				_mm_load_ss(m_rng.floatU01()),
										pr))&1)
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
                        } // if: fresnel reflection
                    }
                    pkt.dir = PacketDirection(SSE::UnitVector3(SSE::Vector3(newdir),SSE::NoCheck));

                } // if: refractive index difference
            } // if: material change


            // changing tetras
            if (IDt_next != IDt)
            {
            	log_event(logger,Events::newTetra,pkt,currTetra,stepResult.idx);

            	if (IDt_next == 0)				// exiting mesh
            	{
            		log_event(logger,Events::exit,make_pair(pkt.p,__m128(pkt.direction())),stepResult.IDfe,pkt.w);
            		return 0;
            	}
            	else							// not exiting mesh
            	{
            		IDt = IDt_next;
            		IDm_next = IDm_bound;
            		currTetra=m_parentKernel.m_mesh->getTetra(IDt);
            	}
            }

            // if material ID changed, fetch new material and update step length remaining
            if (IDm != IDm_next)
            {
                IDm = IDm_next;
                currMat = m_parentKernel.m_mats[IDm];
                pkt.s = _mm_div_ss(_mm_movehdup_ps(pkt.s), _mm_set_ss(currMat.muT()));
            }

            // get the next intersection
            stepResult=currTetra.getIntersection(pkt.p,__m128(pkt.direction()),pkt.s);
            pkt.p   = stepResult.Pe;
        }
        if (Nhit >= m_parentKernel.Nhit_max_)
        {
        	cerr << "Terminated due to unusual number of interface hits at tetra " << IDt
        			<< "  Nhit=" << Nhit << " dimensionless step remaining=" << as_array(pkt.s)[0]  << " w=" << pkt.w << endl;
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

        tie(term,dw)=terminationCheck(m_parentKernel.wmin_,m_parentKernel.prwin_,m_rng,pkt,currMat,currTetra);

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

    	if (scatter(pkt,currMat,currTetra))
    		log_event(logger,Events::scatter,__m128(pkt.direction()),__m128(pkt.direction()),std::numeric_limits<float>::quiet_NaN());
    }

    {
    	AutoStreamBuffer b(cerr);
    	b << "Terminated due to unusual number of steps at tetra " << IDt
			<< " dimensionless step remaining=" << as_array(pkt.s)[0] << " w=" << pkt.w << '\n';
    }

    // should only fall through to here in abnormal circumstances (too many steps)
//    cerr << "Abnormal condition: packet retired after " << Nstep << " steps" << endl;
//    cerr << "p=" << pkt.p << " d=" << __m128(pkt.direction()) << " a=" << pkt.a << " b=" << pkt.b << endl;
//    cerr << "IDt=" << IDt << " |d|=" << norm(__m128(pkt.direction())) << endl;

    log_event(logger,Events::abnormal,pkt,Nstep,Nhit);
    return -1;
}


#endif

