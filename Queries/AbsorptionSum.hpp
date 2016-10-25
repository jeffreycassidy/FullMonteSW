/*
 * AbsorptionSum.hpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_ABSORPTIONSUM_HPP_
#define OUTPUTTYPES_ABSORPTIONSUM_HPP_

#include <FullMonteSW/Geometry/Filters/FilterBase.hpp>
#include <FullMonteSW/OutputTypes/SpatialMap.hpp>

class AbsorptionSum
{
public:
	AbsorptionSum();

	void tetraFilter(const FilterBase<unsigned>* F)									{ m_filter= F ? F : &s_alwaysTrue;	}

	void 							data(const VolumeAbsorbedEnergyMap* E)			{ m_E=E;		}
	const VolumeAbsorbedEnergyMap*	data() 									const 	{ return m_E; 	}

	float compute() const;

private:
	static const AlwaysTrue<unsigned> s_alwaysTrue;

	const VolumeAbsorbedEnergyMap* m_E=nullptr;
	const FilterBase<unsigned>* m_filter=&s_alwaysTrue;
};



#endif /* OUTPUTTYPES_ABSORPTIONSUM_HPP_ */
