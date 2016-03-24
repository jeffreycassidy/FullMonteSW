/*
 * UnitsBase.hpp
 *
 *  Created on: Mar 24, 2016
 *      Author: jcassidy
 */

#ifndef OUTPUTTYPES_UNITSBASE_HPP_
#define OUTPUTTYPES_UNITSBASE_HPP_

#include <boost/serialization/access.hpp>

class UnitsBase
{
public:
	explicit UnitsBase(
			float totalEmitted=0.0f,
			float joulesPerEnergyUnit=std::numeric_limits<float>::quiet_NaN(),
			float cmPerLengthUnit=std::numeric_limits<float>::quiet_NaN()) :
		m_totalEmitted(totalEmitted),
		m_joulesPerEnergyUnit(joulesPerEnergyUnit),
		m_cmPerLengthUnit(cmPerLengthUnit){}

	UnitsBase(const UnitsBase& E) = default;

	void joulesPerEnergyUnit(float j)					{ m_joulesPerEnergyUnit=j; 		}
	float joulesPerEnergyUnit()					const	{ return m_joulesPerEnergyUnit;	}

	void totalEmitted(float E)							{ m_totalEmitted=E;				}
	float totalEmitted()						const	{ return m_totalEmitted;		}

	void cmPerLengthUnit(float l)						{ m_cmPerLengthUnit=l;			}
	float cmPerLengthUnit()						const	{ return m_cmPerLengthUnit;		}

private:
	float 												m_totalEmitted=0.0f;
	float 												m_joulesPerEnergyUnit=std::numeric_limits<float>::quiet_NaN();
	float 												m_cmPerLengthUnit=std::numeric_limits<float>::quiet_NaN();

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{
			nan_wrapper<float> Et(m_totalEmitted,-1.0f,"totalEmitted");
			nan_wrapper<float> Ej(m_joulesPerEnergyUnit,-1.0f,"joulesPerEnergyUnit");
			nan_wrapper<float> Lcm(m_cmPerLengthUnit,0.0f,"cmPerLengthUnit");
			ar & Et & Ej & Lcm;
		}
	friend boost::serialization::access;
};

BOOST_CLASS_IS_WRAPPER(UnitsBase)



#endif /* OUTPUTTYPES_UNITSBASE_HPP_ */
