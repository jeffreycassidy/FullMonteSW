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

class KernelObserver;

namespace Source { class Base; }
class LoggerResults;

class Kernel {
public:
	virtual ~Kernel(){}

	void runSync();
	void startAsync();

	virtual void 	awaitFinish()		=0;
	virtual bool 	done() 				const=0;
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

	const LoggerResults* getResult(std::string,std::string="") const;

	boost::any_range<
		LoggerResults*,
		boost::forward_traversal_tag> results() const { return boost::any_range<LoggerResults*,boost::forward_traversal_tag>(m_results); }

protected:
	const Source::Base*					m_src=nullptr;
	std::vector<SimpleMaterial>			m_materials;

	std::vector<LoggerResults*> 			m_results;

	void addResults(LoggerResults*);
	void clearResults();

private:
	std::vector<KernelObserver*> m_observers;

	virtual void prepare_()=0;
	virtual void start_()=0;

	// scale
	float L_=1.0;

	// total energy launched
	float m_energy=1.0;
};

#endif /* KERNELS_KERNEL_HPP_ */
