/*
 * Kernel.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef KERNELS_KERNEL_HPP_
#define KERNELS_KERNEL_HPP_

#include <vector>
#include <FullMonte/Geometry/SimpleMaterial.hpp>

#include <memory>

#include <boost/range/any_range.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <string>

class KernelObserver;

namespace Source { class Base; }

class OutputData;

class Kernel {
public:
	enum Status { Idle, Preparing, Running, Finished };

	virtual ~Kernel(){}

	void runSync();
	void startAsync();

	void finishAsync();


	bool 	done() const { return m_status == Finished; }
	virtual float 	progressFraction() 	const=0;

	// get/set source (multiple sources accommodated by Source::Composite)
	void				source(const Source::Base* s)			{ m_src=s;			}
	const Source::Base*	source()						const 	{ return m_src; 	}

	// get/set total energy emitted
	void				energy(float E)							{ m_energy=E;		}
	float				energy()						const	{ return m_energy; 	}

	void 								materials(const std::vector<SimpleMaterial>& mats)	{ m_materials=mats; 	}
	const std::vector<SimpleMaterial>& 	materials()			const							{ return m_materials;	}

	// length of simulation unit, measured in cm
	void setUnitsToCM(){ L_=1.0;  	}
	void setUnitsToMM(){ L_=0.1; 	}

	const OutputData* getResultByTypeString(std::string) const;

	template<class Result>const Result* getResultByType() const
	{
		const Result* r=nullptr;
		for(unsigned i=0; i<m_results.size() && !r; ++i)
			r=dynamic_cast<const Result*>(m_results[i]);

		return r;
	}

	boost::any_range<
		OutputData*,
		boost::forward_traversal_tag> results() const { return boost::any_range<OutputData*,boost::forward_traversal_tag>(m_results); }

protected:
	const Source::Base*					m_src=nullptr;
	std::vector<SimpleMaterial>			m_materials;

	std::vector<OutputData*> 			m_results;

	void addResults(OutputData*);
	void clearResults();
	void awaitStatus(Status st);
	void updateStatus(Status st);

private:

	virtual void 	awaitFinish()		=0;

	std::vector<KernelObserver*> m_observers;

	virtual void prepare_()=0;
	virtual void start_()=0;
	virtual void postfinish()=0;

	Status						m_status=Idle;
	std::mutex					m_statusMutex;
	std::condition_variable 	m_statusCV;

	std::thread 				m_parentThread;

	// scale
	float L_=1.0;

	// total energy launched
	float m_energy=1.0;
};

#endif /* KERNELS_KERNEL_HPP_ */
