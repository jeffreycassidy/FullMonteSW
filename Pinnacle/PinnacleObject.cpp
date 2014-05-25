#include "PinnacleObject.hpp"

#include <sstream>
#include <iostream>

namespace Pinnacle {

void Object::addprop(string k,string v)
{
	props.insert(make_pair(k,v));
}

template<>string Object::getprop(string propname) const
{
	return props.at(propname);
}

}
