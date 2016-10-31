#ifndef ABSTRACTSPATIALMAP_INCLUDED_
#define ABSTRACTSPATIALMAP_INCLUDED_
#include <vector>

#include "clonable.hpp"
#include "visitable.hpp"

#include <FullMonteSW/Geometry/Units/Unit.hpp>
#include <FullMonteSW/Geometry/Units/Quantity.hpp>

#include "OutputData.hpp"

using namespace std;

/** Represents a spatial map of some property over elements in a geometry */

class AbstractSpatialMap : public OutputData
{
public:
	enum SpaceType { UnknownSpaceType=-1, Point=0, Line, Surface, Volume };
	enum ValueType { Scalar, Vector, UnknownValueType=255 };

	explicit AbstractSpatialMap(SpaceType type=UnknownSpaceType,ValueType=Scalar);
	virtual ~AbstractSpatialMap();

	virtual void			dim(std::size_t N)=0;
	virtual std::size_t		dim() const=0;

	SpaceType spatialType() const { return m_spaceType; }
	ValueType valueType() const { return m_valueType; }

	const Units::Quantity*		quantity() const	{ return m_quantity;	 	}
	const Units::Unit*			units() 	const 	{ return m_units; 			}

	void				quantity(const Units::Quantity* Q){ m_quantity=Q; }
	void				units(const Units::Unit* U){ m_units=U; }

private:
	SpaceType					m_spaceType=UnknownSpaceType;
	ValueType					m_valueType=UnknownValueType;

	const Units::Quantity*		m_quantity=nullptr;
	const Units::Unit*			m_units=nullptr;
};

#endif
