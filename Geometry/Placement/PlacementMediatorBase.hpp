/*
 * PlacementMediatorBase.hpp
 *
 *  Created on: Mar 9, 2016
 *      Author: jcassidy
 */

#ifndef GEOMETRY_PLACEMENT_PLACEMENTMEDIATORBASE_HPP_
#define GEOMETRY_PLACEMENT_PLACEMENTMEDIATORBASE_HPP_

class PlacementBase;

/** Mediator between a placement constraint and a source definition.
 *
 * Concrete instances are responsible for setting the underlying source's parameters when update() is called.
 */

namespace Source { class Abstract; }


class PlacementMediatorBase
{
public:
	virtual ~PlacementMediatorBase();

	void					placement(PlacementBase* p);
	PlacementBase*			placement()							const	{ return m_placement;	}

	void					source(Source::Abstract* s)						{ m_source=s;			}
	Source::Abstract*			source()							const	{ return m_source;		}

	virtual void			update()=0;

private:
	PlacementBase*		m_placement=nullptr;
	Source::Abstract*		m_source=nullptr;
};


template<class PlacementT,class SourceT>class PlacementMediatorInstance : public PlacementMediatorBase
{
public:
	using PlacementMediatorBase::placement;
	using PlacementMediatorBase::source;

	PlacementT*	placement()						const	{ return static_cast<PlacementT*>(PlacementMediatorBase::placement()); 		}
	SourceT*	source() 						const	{ return static_cast<SourceT*>(PlacementMediatorBase::source());			}
};




#endif /* GEOMETRY_PLACEMENT_PLACEMENTMEDIATORBASE_HPP_ */
