#include "DVH.hpp"
#include "graph.hpp"
#include "Parallelepiped.hpp"

#include <boost/serialization/vector.hpp>

template<typename T>vector<T> loadTextVector(string fn)
{
	ifstream is(fn);
	vector<T> v;
	T t;
	for(is >> t; !is.eof() && !is.fail(); is >> t)
		v.push_back(t);
	return v;
}

int main(int argc,char **argv)
{
	unsigned IDtarget=20;

	// get the mesh
	string meshFn("data/mouse.mesh");
	string fluenceFn("DBUtils/out.3093.phi_v.txt");

	cout << "Mesh: reading " << meshFn << endl;
	TetraMesh M(meshFn,TetraMesh::MatlabTP);

	// read list of tumour elements
	vector<unsigned> regions = M.getMaterialMap();
	vector<unsigned> tumour;


	string targetFn("tumour_tet_IDs.txt");
	ifstream is(targetFn.c_str());
	copy(std::istream_iterator<unsigned>(is),std::istream_iterator<unsigned>(),std::back_inserter(tumour));

	cout << "Target: Read " << tumour.size() << " elements from " << targetFn << endl;

	for(unsigned IDt : tumour)
		regions[IDt] = IDtarget;

	// set bounding box
	Parallelepiped dvhbox(std::array<std::pair<UnitVector<3,double>,std::pair<double,double>>,3>{
		make_pair(UnitVector<3,double>{1,0,0},make_pair(8,28)),
		make_pair(UnitVector<3,double>{0,1,0},make_pair(33,48)),
		make_pair(UnitVector<3,double>{0,0,1},make_pair(4,19))
		});

	// if tetra has no points inside the bounding box then set its region to zero
	// yes, it's <= because there's a zero element prepended so the vector size is +1
	for(unsigned i=0;i<=M.getNt();++i)
	{
		bool inside=false;
		for(unsigned j=0;j<4;++j)
			inside |= dvhbox.pointWithin(M.getTetraPoint(i,j));

		if(!inside)
			regions[i]=0;
	}


	// load the fluence
	vector<double> energy = loadTextVector<double>(fluenceFn);
	vector<double> fluence(energy.size(),0.0);

	vector<double>::const_iterator 	energy_it=energy.begin();
	vector<double>::iterator		fluence_it=fluence.begin();

	double sum=0.0;
	for(double v : fluence)
		sum += v;
	cout << "Total energy found: " << sum << endl;

	// do the DVH
	DVH dvh(M,fluence,regions);
	dvh.writeASCII("dvh.out");
	//cout << dvh << endl;
}
