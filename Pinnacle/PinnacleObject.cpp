#include "PinnacleObject.hpp"

#include <sstream>
#include <iostream>

namespace Pinnacle {

void Object::addprop(string k,string v)
{
	props.insert(make_pair(k,v));
}

template<>string Object::convert(const string& str,bool){ return str; }

pair<bool,string> Object::__getprop(const string& propname) const
{
	map<string,string>::const_iterator it=props.find(propname);
	return ((it==props.end()) ? make_pair(false,string()) : make_pair(true,it->second));
}

}
