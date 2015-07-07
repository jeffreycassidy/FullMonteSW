#include <string>
#include <vector>
#include <array>

#include "TetGenBase.hpp"
#include <FullMonte/Geometry/TetraMeshBase.hpp>


namespace TetGen {

class TetGenReader : public TetGenBase {

public:
	TetGenReader(){}
	TetGenReader(const std::string pfx) : TetGenBase(pfx){}

	TetraMeshBase mesh() const;

	// doesn't do materials or sources

private:
	std::vector<std::array<double,3> > 										load_node_file(std::string fn) const;
	std::pair<std::vector<std::array<unsigned,4> >,std::vector<unsigned> > 	load_ele_file(std::string fn) const;

};


};
