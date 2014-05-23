#pragma once
#include <map>
#include <string>

using namespace std;

namespace Pinnacle {

class Object {
	map<string,string> props;

public:

	// add properties
	void addprop(string,string);

	template<typename T>T getprop(string) const;
};


}
