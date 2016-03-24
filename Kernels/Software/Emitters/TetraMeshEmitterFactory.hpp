/*
 * EmitterFactory.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_EMITTERS_TETRAMESHEMITTERFACTORY_HPP_
#define KERNELS_SOFTWARE_EMITTERS_TETRAMESHEMITTERFACTORY_HPP_

#include <FullMonte/Geometry/Sources/Visitor.hpp>

#include <boost/range.hpp>
#include <boost/range/any_range.hpp>

#include <boost/range/adaptor/map.hpp>

#include <vector>

class TetraMesh;

namespace Emitter
{

template<typename RNG>class EmitterBase;

/** TetraEmitterFactory takes generic source descriptions (descendents of Source::Base) and makes them into
 * emitters that can be used with the tetrahedral MC kernel.
 *
 * It inherits from Source::Visitor to generalize over source types.
 */

template<class RNG>class TetraEmitterFactory : public Source::Visitor
{

public:
	TetraEmitterFactory(){}
	TetraEmitterFactory(const TetraMesh* M) : m_mesh(M){}

	virtual void visit(Source::PointSource* p) 	override;
	virtual void visit(Source::Ball* b)			override;
	virtual void visit(Source::Line* l)			override;
	virtual void visit(Source::Volume* v)		override;
	virtual void visit(Source::Composite* c)	override;
	virtual void visit(Source::Surface* s)		override;
	virtual void visit(Source::SurfaceTri* st)	override;
	virtual void visit(Source::Base* b)			override;
	virtual void visit(Source::PencilBeam* b)	override;

	Emitter::EmitterBase<RNG>* emitter() const;

	boost::iterator_range<std::vector<Source::Base*>::const_iterator> csources() const
		{
			return boost::iterator_range<std::vector<Source::Base*>::const_iterator>(m_sources.cbegin(), m_sources.cend());
		}

	boost::any_range<
		Emitter::EmitterBase<RNG>*,
		boost::random_access_traversal_tag> cemitters() const
			{
				return m_emitters | boost::adaptors::map_values;
			}

private:
	std::vector<Source::Base*> 									m_sources;
	std::vector<std::pair<float,Emitter::EmitterBase<RNG>*>>	m_emitters;
	const TetraMesh*											m_mesh=nullptr;

	bool														m_debug=true;
};

};


#endif /* KERNELS_SOFTWARE_EMITTERS_TETRAMESHEMITTERFACTORY_HPP_ */
