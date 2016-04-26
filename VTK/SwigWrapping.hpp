/*
 * SwigWrapping.hpp
 *
 *  Created on: Feb 19, 2016
 *      Author: jcassidy
 */

#ifndef VTK_SWIGWRAPPING_HPP_
#define VTK_SWIGWRAPPING_HPP_

struct SwigPointerInfo {
	void* 								p;
	std::pair<const char*,const char*>	type;
};

template<typename T>T endianSwap(T i);

template<>uint64_t endianSwap(uint64_t i)
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

	assert(*s == '_');

	const char *pStart=s+1;
	const char *pEnd=s+1;
	for(pEnd=s+1; *pEnd != '\0' && *pEnd != '_';++pEnd){}

	sscanf(pStart,"%llx",&ptr);

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!
	info.p=reinterpret_cast<void*>(endianSwap(ptr));

	const char *tStart=pEnd, *tEnd=pEnd;

	assert(*(tStart++)=='_');
	assert(*(tStart++)=='p');
	assert(*(tStart++)=='_');

	for(tEnd=tStart+1; tEnd != '\0';++tEnd){}

	info.type = make_pair(tStart,tEnd);

	std::cout << "typeStr='" << std::string(tStart,tEnd) << "'" << std::endl;
	return info;
}

void SparseVectorVTK::setInputSparseVector(const char* pStr)
{
	// kludgy, home-brewed pointer extraction
	// format is _XXXXXXXXX_p_TTTTTT where X is pointer value (little-endian hex) and T is type
	std::stringstream ss(pStr);
	std::string typeStr;
	char buf[5];
	uint64_t pval;
	ss.read(buf,1);
	ss >> std::hex >> pval;
	ss.read(buf+1,3);
	buf[4]=0;
	if (strcmp("__p_",buf))
		std::cout << "ERROR: Improperly formatted string" << std::endl;

	ss >> typeStr;
	std::cout << "typeStr='" << typeStr << "'" << std::endl;

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!
	pval = (pval >> 32) | (pval << 32);
	pval = ((pval >> 16) & 0x0000ffff0000ffffULL) | ((pval << 16) & 0xffff0000ffff0000ULL);
	pval = ((pval >> 8 ) & 0x00ff00ff00ff00ffULL) | ((pval << 8)  & 0xff00ff00ff00ff00ULL);

	m_sv = reinterpret_cast<const SparseVector<unsigned,float>*>(pval);
	update();
}

uint64_t endianSwap(uint64_t i)
{
	uint64_t pval=i;

	// For some reason, SWIG stores pointers in little-endian strings; have to swap them before using!
	pval = (pval >> 32) | (pval << 32);
	pval = ((pval >> 16) & 0x0000ffff0000ffffULL) | ((pval << 16) & 0xffff0000ffff0000ULL);
	pval = ((pval >> 8 ) & 0x00ff00ff00ff00ffULL) | ((pval << 8)  & 0xff00ff00ff00ff00ULL);

	return pval;
}

void TetraMeshVTK::setInputTetraMesh(const char* pStr)
{
	unsigned i=1;

	//if (strncmp(pStr,"_",1) || strncmp(pStr+))

	// kludgy, home-brewed pointer extraction
	// format is _XXXXXXXXX_p_TTTTTT where X is pointer value (little-endian hex) and T is type
	std::stringstream ss(pStr);
	std::string typeStr;
	char buf[5];
	uint64_t pval;
	ss.read(buf,1);
	ss >> std::hex >> pval;
	ss.read(buf+1,3);
	buf[4]=0;
	if (strcmp("__p_",buf))
		std::cout << "ERROR: Improperly formatted string" << std::endl;
	ss >> typeStr;

	m_M = reinterpret_cast<const TetraMesh*>(endianSwap(pval));
	update();
}



#endif /* VTK_SWIGWRAPPING_HPP_ */
