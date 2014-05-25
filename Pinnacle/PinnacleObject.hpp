#pragma once
#include <map>
#include <string>

#include <sstream>
#include <iostream>

using namespace std;

namespace Pinnacle {

class Object {
	map<string,string> props;

public:

	typedef map<string,string>::const_iterator prop_const_iterator;
	typedef pair<prop_const_iterator,prop_const_iterator> prop_const_range;

	// add properties
	void addprop(string,string);

	template<typename T>T getprop(string) const;
	//template<typename T>T getprop(const char* s) const { return getprop<T>(string(s)); }

	prop_const_range getPropRange() const { return make_pair(props.begin(),props.end()); }
};

template<typename T>T Object::getprop(string propname) const
{
	stringstream ss(props.at(propname));
	unsigned t;
	ss >> t;
	if (ss.fail())
		cerr << "Parse error: parse failed" << endl;
	else if(!ss.eof())
		cerr << "Parse error: trailing characters" << endl;
	return t;
}

// template specializations
template<>string Object::getprop(string propname) const;

}
