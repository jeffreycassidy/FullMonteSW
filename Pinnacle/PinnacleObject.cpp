#include "PinnacleObject.hpp"

namespace Pinnacle {

void Object::addprop(string k,string v)
{
	props.insert(make_pair(k,v));
}

template<>double Object::getprop(string propname) const
{
	string s = props.at(propname);
}

template<>string Object::getprop(string propname) const
{
	return props.at(propname);
}

template<>unsigned Object::getprop(string propname) const
{
	string s=props.at(propname);
}

}
