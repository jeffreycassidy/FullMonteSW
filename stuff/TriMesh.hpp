#include "newgeom.hpp"
#include "graph.hpp"
#include <vector>

using namespace std;

class TriMesh {
    vector<Point<3,double> >    P;
    vector<FaceByPointID>       F;

	vector<double> scalars;

    public:

    void fromVTKFile(string);
	vector<int> matchTo(const TetraMesh&);

	void writePoints(string fn) const;

	friend class TetraMesh;
};
