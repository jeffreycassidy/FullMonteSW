/*
 * UnitsTest.cpp
 *
 *  Created on: Sep 22, 2016
 *      Author: jcassidy
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE UnitsTest

#include <boost/test/unit_test.hpp>

#include "Unit.hpp"
#include "UnitPrefix.hpp"
#include "PrefixedUnit.hpp"

#include "BaseUnit.hpp"

#include <iostream>

using namespace std;
using namespace Units;

//// Check that unit lhs can be converted into k rhs units

#define CHECK_CONVERTS(lhs,rhs,k) cout << "[" << lhs.symbol() << "] can" << (DerivedUnit::convertible(lhs,rhs) ? "" : "'t") << " be converted to " << DerivedUnit::convert(lhs,rhs) << " [" << rhs.symbol() << "]" << endl;\
	BOOST_CHECK_CLOSE(DerivedUnit::convert(lhs,rhs),k,1e-3f);			\
	BOOST_CHECK(DerivedUnit::convertible(lhs,rhs));

#define CHECK_NO_CONVERT(lhs,rhs) cout << "[" << lhs.symbol() << "] can" << (DerivedUnit::convertible(lhs,rhs) ? "" : "'t") << " be converted to [" << rhs.symbol() << "]" << endl;\
	BOOST_CHECK(!DerivedUnit::convertible(lhs,rhs));

BOOST_AUTO_TEST_CASE(mmToM)
{
	PrefixedUnit mm{&Units::milli,&metre};
	PrefixedUnit cm{&Units::centi,&metre};

	cout << "The " << mm.name() << " is equivalent to " << mm.scale() << " " << mm.base()->name() << endl;

	DerivedUnit mmc = mm.canonicalize();

	cout << "  canonical form: " << mmc.symbol() << endl;

	DerivedUnit mm2 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 2, &mm } } };
	DerivedUnit cm2 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 2, &cm } } };
	DerivedUnit fcm2 { 1e-4f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 2, &metre } } };
	DerivedUnit cm3 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 3, &cm } } };

	DerivedUnit mm2c = mm2.canonicalize();

	cout << "DerivedUnit mm2 has units of [" << mm2.symbol() << "] and canonical units [" << mm2.canonicalize().symbol() << "]" << endl;

	cout << "  canonical unit [" << mm2c.symbol() << "]" << endl;


	DerivedUnit mm3 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component {3, &mm } } };
	DerivedUnit mm3c = mm3.canonicalize();

	BaseUnit J{"Joule","J"};

	cout << "DerivedUnit mm3 has units of [" << mm3.symbol() << "] and canonical units [" << mm3.canonicalize().symbol() << "]" << endl;

		cout << "  canonical unit [" << mm3c.symbol() << "]" << endl;


	DerivedUnit Jmm2 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 1, &J }, DerivedUnit::Component { -2, &mm }}};
	cout << "Fluence is in units of [" << Jmm2.symbol() << "]" << endl;
	cout << "  Canonicalizes to [" << Jmm2.canonicalize().symbol() << "]" << endl;

	DerivedUnit Jcm2 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 1, &J }, DerivedUnit::Component {-2, &cm }}};
	DerivedUnit Jcm3 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { 1, &J }, DerivedUnit::Component {-3, &cm }}};
	DerivedUnit Jmm3 { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { -3, &mm }, DerivedUnit::Component {1, &J }}};

	CHECK_CONVERTS(Jcm2,Jmm2,0.01f)
	CHECK_CONVERTS(Jcm2,Jcm2, 1.0f)
	CHECK_CONVERTS(Jcm3,Jmm3,0.001f)
	CHECK_CONVERTS(fcm2,cm2, 1.0f)
	CHECK_NO_CONVERT(Jcm2,mm2)
	CHECK_NO_CONVERT(Jmm2,mm3)

	BOOST_CHECK_CLOSE(DerivedUnit::convert(Jcm2,Jmm2),0.01f,1e-3f);
	BOOST_CHECK_CLOSE(DerivedUnit::convert(mm2,cm2),0.01f,1e-3f);
	BOOST_CHECK_CLOSE(DerivedUnit::convert(cm3,mm3),1000.0f,1e-3f);


	//cout << "The " << metre.name() << " is a unit of " << metre.quantity()->name() << " equivalent to " << 1.0f/mm.scale() << " " << mm.name() << 's' << endl;

	// DerivedUnit mm2{&area, &mm, 2};

	//cout << "The " << mm2.name() << " is a unit of " << mm2.quantity()->name() << " equivalent to " << mm2.scale() << " " << endl;
}

BOOST_AUTO_TEST_CASE(fluence)
{
	DerivedUnit J { 1.0f, std::vector<DerivedUnit::Component>{
		DerivedUnit::Component { 1, &kilogram },
		DerivedUnit::Component { 2, &metre },
		DerivedUnit::Component { -2, &second }}};

	cout << "The Joule is defined as 1 [" << J.symbol() << "]" << endl;

	DerivedUnit W = J * DerivedUnit { 1.0f, std::vector<DerivedUnit::Component>{ DerivedUnit::Component { -1, &second }}};

	cout << "  which makes the Watt equal 1 [" << W.symbol() << "]" << endl;
	cout << "  or canonically 1 [" << W.canonicalize().symbol() << "]" << endl;

	DerivedUnit Newton { 1.0f, std::vector<DerivedUnit::Component>{
		DerivedUnit::Component { 1, &kilogram },
		DerivedUnit::Component { 1, &metre },
		DerivedUnit::Component { -2, &second }}};

	DerivedUnit Nm = Newton*metre;

	CHECK_CONVERTS(Nm,J,1.0f);
}
