/*
 * TetraVolumeKernel.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#include "TetraVolumeKernel.hpp"
#include "TetraMCKernelThread.hpp"

template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

void TetraVolumeKernel::postfinish()
{
	auto res = std::make_tuple(
			get<0>(m_logger).getResults(),
			get<1>(m_logger).getResults(),
			get<2>(m_logger).getResults());

	// clone because storage above is of automatic duration
	tuple_for_each(res, [this] (const LoggerResults& r)
			{ addResults(r.clone()); });

	cout << "Results are available" << endl;

	for(auto p : results())
		p->summarize(cout);

	cout << endl << endl << "Result types available: ";
	for(auto p : results())
		cout << " " << p->getTypeString();
	cout << endl;

	cout << "Kernel is finished" << endl;
}

