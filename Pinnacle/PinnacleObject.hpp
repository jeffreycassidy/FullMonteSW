#pragma once
#include <map>
#include <string>

#include <sstream>
#include <iostream>

using namespace std;

namespace Pinnacle {

class Object {
	map<string,string> props;

	pair<bool,string> __getprop(const string&) const; ///< Gets a property, returning <true,value> if found else <false,X>

	template<typename T>static T convert(const string&,bool exc_=false);

public:

	typedef map<string,string>::const_iterator prop_const_iterator;
	typedef pair<prop_const_iterator,prop_const_iterator> prop_const_range;

	// add properties
	void addprop(string,string);

	template<typename T>T getprop(const string&) const;		///< Gets a property, throwing an exception
	template<typename T>T getprop_default(const string&,const T&) const;	///< Gets a property, returning a default value if not present

	prop_const_range getPropRange() const { return make_pair(props.begin(),props.end()); }
};

template<typename T>T Object::getprop(const string& propname) const
{
	pair<bool,const string&> p = __getprop(propname);

	if (!p.first)
		throw string("ERROR: Expected property not found");
	else
		return convert<T>(p.second);
}

template<typename T>T Object::convert(const string& str,bool exc_)
{
	stringstream ss(str);
	T t=T();
	ss >> t;
	if (ss.fail())
	{
		if (exc_)
			throw string("Parse error: parse failed");
		else
			cerr << "Parse error: parse failed on string \"" << str << "\"" << endl;
	}
	else if(!ss.eof())
	{
		if (exc_)
			throw string("Parse error: trailing characters");
		else
			cerr << "Parse error: trailing characters: \"" << ss.str() << "\"" << endl;
	}
	return t;
}

template<>string Object::convert(const string& str,bool);

template<typename T>T Object::getprop_default(const string& propname,const T& defval) const
{
	pair<bool,const string&> p = __getprop(propname);
	return p.first ? convert<T>(p.second) : defval;
}

}
