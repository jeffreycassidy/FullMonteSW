#include <vector>
#include <array>
#include <string>
#include <boost/static_assert.hpp>

namespace STL {
namespace Binary {

#ifndef SWIG
struct Header {
	uint8_t 	comm[80];
	uint32_t 	Ntri;
} __attribute__((packed));

struct Entry {
	std::array<float,3> 				n;
	std::array<std::array<float,3>,3> 	p;
	uint16_t 							attr;
} __attribute__((packed));

// make sure that the packed arrays actually pack
BOOST_STATIC_ASSERT(sizeof(Entry)==50);
BOOST_STATIC_ASSERT(sizeof(Header)==84);
BOOST_STATIC_ASSERT(sizeof(std::array<float,3>)==3*sizeof(float));
BOOST_STATIC_ASSERT(sizeof(float)==4);
#endif

std::pair<std::vector<std::array<float,3>>,std::vector<std::array<unsigned,3>>> loadSTLBinary(std::string fn);

}
}
