ADD_EXECUTABLE(UnitsTest Unit.cpp UnitsTest.cpp Unit.cpp BaseUnit.cpp PrefixedUnit.cpp UnitPrefix.cpp Quantity.cpp)
TARGET_LINK_LIBRARIES(UnitsTest ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
