/*
 * TetraMCKernel.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_TETRAMCKERNEL_HPP_
#define KERNELS_SOFTWARE_TETRAMCKERNEL_HPP_

#include <FullMonte/Geometry/Sources/Print.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <FullMonte/Kernels/TetraKernelBase.hpp>
#include <FullMonte/Kernels/Software/ThreadedMCKernelBase.hpp>
#include <FullMonte/Kernels/Software/Emitters/TetraMeshEmitterFactory.cpp>

#include "Material.hpp"

template<class RNG>class TetraMCKernel : public ThreadedMCKernelBase, public TetraKernelBase
{
public:
	TetraMCKernel(const TetraMesh* mesh) :
		ThreadedMCKernelBase(),
		TetraKernelBase(mesh)
	{}

	template<class Logger>class Thread;

	virtual ThreadedMCKernelBase::Thread* makeThread() override=0;

protected:
	virtual void parentPrepare() 		override;

	std::vector<Material> mat_;

	Emitter::EmitterBase<RNG>* m_emitter;
};

extern template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;

template<class RNG>void TetraMCKernel<RNG>::parentPrepare()
{
	if (!m_src)
	{
		cerr << "ERROR: No source specified" << endl;
		throw std::logic_error("TetraMCKernel<RNG>::prepare_ no sources specified");
	}

	if (!m_mesh)
	{
		cerr << "ERROR: No mesh specified" << endl;
		throw std::logic_error("TetraMCKernel<RNG>::prepare_ no mesh specified");
	}

	// copy sources
	cout << "Source: " << *m_src << endl;

	Emitter::TetraEmitterFactory<RNG> factory(m_mesh);

	((Source::Base*)m_src)->acceptVisitor(&factory);

	m_emitter = factory.emitter();

	mat_.resize(m_materials.size());

	// copy materials
	boost::copy(
		m_materials | boost::adaptors::transformed(std::function<Material(SimpleMaterial)>([](SimpleMaterial sm){ return Material(sm.mu_a,sm.mu_s,sm.g,sm.n); })),
		mat_.begin());

	cout << "Materials: " << endl;
	for(const Material& m : mat_)
		cout << m << endl;

	std::vector<unsigned> hist;
	for(unsigned i=0; i<=m_mesh->getNt(); ++i)
	{
		unsigned mat = m_mesh->getMaterial(i);
		if (mat >= hist.size())
			hist.resize(mat+1);
		hist[mat]++;
	}

	cout << "Material | Tetra Count" << endl;
	unsigned i=0;
	for(auto h : hist)
		cout << ++i << ": " << h << endl;
}


#endif /* KERNELS_SOFTWARE_TETRAMCKERNEL_HPP_ */
