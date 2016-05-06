/*
 * SwigWrapping.cpp
 *
 *  Created on: Apr 26, 2016
 *      Author: jcassidy
 */

#include "SwigWrapping.hpp"

#include <cinttypes>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstring>

using namespace std;

uint64_t endianSwap(uint64_t i)
{
	i = (i >> 32) | (i << 32);
	i = ((i >> 16) & 0x0000ffff0000ffffULL) | ((i << 16) & 0xffff0000ffff0000ULL);
	i = ((i >> 8 ) & 0x00ff00ff00ff00ffULL) | ((i << 8)  & 0xff00ff00ff00ff00ULL);
	return i;
}



SwigPointerInfo readSwigPointer(const char *s)
{
	// format is _XXXXXXXXX_p_TTTTTT where X is pointer value (little-endian hex) and T is type
	uint64_t ptr;
	SwigPointerInfo info;

	if (*s != '_')
		return info;

	const char *pStart=s+1,*pEnd;
	for(pEnd=pStart; *pEnd != '\0' && *pEnd != '_';++pEnd){}

	sscanf(pStart,"%lx",&ptr);

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!

	const char *tStart=pEnd, *tEnd=pEnd;

	if(strncmp(tStart,"_p_",3))
		return info;
	tStart += 3;

	for(tEnd=tStart+1; *tEnd != '\0';++tEnd){}

	info.p=reinterpret_cast<void*>(endianSwap(ptr));
	info.type = make_pair(tStart,tEnd);
	return info;
}



