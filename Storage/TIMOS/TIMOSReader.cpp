/*
 * TIMOS.cpp
 *
 *  Created on: Mar 3, 2015
 *      Author: jcassidy
 */

#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <sstream>

#include <unordered_map>


using namespace std;

std::unordered_map<int,string> TIMOSParser::toks{
#include "TIMOS_tokens.h"
};
