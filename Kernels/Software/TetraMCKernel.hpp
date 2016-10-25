/*
 * TetraMCKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRAMCKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRAMCKERNEL_HPP_

#include <FullMonteSW/Geometry/Sources/Print.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include "Logger/AbstractScorer.hpp"

#include <FullMonteSW/Kernels/TetraKernelBase.hpp>
#include <FullMonteSW/Kernels/Software/ThreadedMCKernelBase.hpp>
#include <FullMonteSW/Kernels/Software/Emitters/TetraMeshEmitterFactory.cpp>

#include <boost/align/aligned_alloc.hpp>

#include "RNG_SFMT_AVX.hpp"

template<class RNGT,class ScorerT>class TetraMCKernel : public ThreadedMCKernelBase, public TetraKernelBase
{
public:
	TetraMCKernel(const TetraMesh* mesh) :
		ThreadedMCKernelBase(),
		TetraKernelBase(mesh)
	{}

	typedef RNGT										RNG;
	class 												Thread;
	typedef ScorerT 									Scorer;

#ifndef SWIG
	typedef decltype(get_logger(*static_cast<ScorerT*>(nullptr))) 	Logger;
#endif

protected:
	virtual ThreadedMCKernelBase::Thread* makeThread() final override;

	virtual void parentPrepare() 						override;
	virtual std::list<OutputData*> gatherResults()		const override;

	std::vector<Material> m_mats;

	Emitter::EmitterBase<RNG>* 	m_emitter;

	Scorer						m_scorer;
};

extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

template<class RNGT,class Scorer>void TetraMCKernel<RNGT,Scorer>::parentPrepare()
{
	if (!m_src)
	{
		cerr << "ERROR: No source specified" << endl;
		throw std::logic_error("TetraMCKernel<RNGT,Scorer>::parentPrepare no sources specified");
	}

	if (!m_mesh)
	{
		cerr << "ERROR: No mesh specified" << endl;
		throw std::logic_error("TetraMCKernel<RNGT,Scorer>::parentPrepare no mesh specified");
	}

	clear(m_scorer);

	Emitter::TetraEmitterFactory<RNGT> factory(m_mesh);

	((Source::Abstract*)m_src)->acceptVisitor(&factory);

	m_emitter = factory.emitter();

	m_mats.resize(m_materials.size());

	// copy materials
	boost::copy(
		m_materials,
		m_mats.begin());

	std::vector<unsigned> hist;
	for(unsigned i=0; i<=m_mesh->getNt(); ++i)
	{
		unsigned mat = m_mesh->getMaterial(i);
		if (mat >= hist.size())
			hist.resize(mat+1);
		hist[mat]++;
	}
}

template<class RNGT,class Scorer>std::list<OutputData*> TetraMCKernel<RNGT,Scorer>::gatherResults() const
{
	return ::results(m_scorer);
}


template<class RNGT,class Scorer>ThreadedMCKernelBase::Thread* TetraMCKernel<RNGT,Scorer>::makeThread()
{
	void *p = boost::alignment::aligned_alloc(32,sizeof(typename TetraMCKernel<RNG_SFMT_AVX,Scorer>::Thread));

	if (!p)
		throw std::bad_alloc();

	// create the thread-local state
	typename TetraMCKernel<RNG_SFMT_AVX,Scorer>::Thread* t = new typename TetraMCKernel<RNG_SFMT_AVX,Scorer>::Thread(*this,get_logger(m_scorer));

	// seed its RNG
	t->seed(TetraMCKernel<RNG_SFMT_AVX,Scorer>::getUnsignedRNGSeed());

	return t;
}

#endif /* KERNELS_SOFTWARE_TETRAMCKERNEL_HPP_ */


