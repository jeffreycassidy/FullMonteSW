/*
 * Kernel.hpp
 *
 *  Created on: Apr 22, 2015
 *      Author: jcassidy
 */

#ifndef KERNELS_KERNEL_HPP_
#define KERNELS_KERNEL_HPP_

#include <vector>
#include <FullMonteSW/Geometry/SimpleMaterial.hpp>

#include <memory>

#include <boost/range/any_range.hpp>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <string>
#include <list>

class KernelObserver;

namespace Source { class Abstract; }

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
	void				source(const Source::Abstract* s)			{ m_src=s;			}
	const Source::Abstract*	source()						const 	{ return m_src; 	}

	// get/set total energy emitted
	void				energy(float E)							{ m_energy=E;		}
	float				energy()						const	{ return m_energy; 	}

	void 								materials(const std::vector<SimpleMaterial>& mats)	{ m_materials=mats; 	}
	const std::vector<SimpleMaterial>& 	materials()			const							{ return m_materials;	}

	// length of simulation unit, measured in cm
	void setUnitsToCM(){ L_=1.0;  	}
	void setUnitsToMM(){ L_=0.1; 	}

	std::size_t 		getResultCount() const
	{
		return m_results.size();
	}

	const OutputData* getResultByIndex(std::size_t i) const
	{
		auto it = m_results.begin();
		std::advance(it,i);
		return *it;
	}

	const OutputData* getResultByTypeString(std::string) const;

	template<class Result>const Result* getResultByType() const
	{
		for(auto r : m_results)
		{
			const Result* p = dynamic_cast<const Result*>(r);
			if (p)
				return p;
		}

		return nullptr;
	}

	boost::any_range<
		OutputData*,
		boost::forward_traversal_tag> results() const { return boost::any_range<OutputData*,boost::forward_traversal_tag>(m_results); }

protected:
	const Source::Abstract*					m_src=nullptr;
	std::vector<SimpleMaterial>			m_materials;

	std::list<OutputData*> 				m_results;

	void awaitStatus(Status st);
	void updateStatus(Status st);

private:

	virtual void 	awaitFinish()		=0;

	std::vector<KernelObserver*> m_observers;

	virtual std::list<OutputData*> gatherResults() const=0;

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

