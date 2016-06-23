#include "VTKHelpers.hpp"

#include <boost/range/adaptor/map.hpp>
#include <boost/range.hpp>
#include <FullMonteSW/Storage/STL/STLBinary.hpp>

using namespace std;

int main(int argc,char **argv)
{
	string fn,ofn;
	if (argc < 2)
	{
		cerr << "Error: filename required" << endl;
		return -1;
	}
	else
		fn = argv[1];

	if (argc < 3)
	{
		cout << "INFO: Default output file name" << endl;
		ofn = "mesh.text.stl";
	}
	else
		ofn = argv[2];

	cout << "Reading from binary STL " << fn << " and saving to text STL " << ofn << endl;

	vector<array<float,3>> P;
	vector<array<unsigned,3>> T;

	tie(P,T) = loadSTLBinary(argv[1]);

	vtkPoints *pts = vtkPoints::New();
	boost::for_each(P, vtk_point_inserter(pts));

	vtkCellArray *ca = vtkCellArray::New();
	boost::for_each(T, vtk_cell_inserter(ca));

	vtkUnsignedCharArray * r = vtkUnsignedCharArray::New();
	r->SetNumberOfTuples(T.size());

	auto __r = T | boost::adaptors::indexed(0U);

	for(auto it=begin(__r); it !=end(__r); ++it)
	{
	}

	vtkPolyData *pd = vtkPolyData::New();

	pd->SetPoints(pts);
	pd->SetPolys(ca);


	write_vtk_file("from_stl.vtk","converted from STL",pd);

	writeSTLText(ofn,P,T,"phantom");

}
