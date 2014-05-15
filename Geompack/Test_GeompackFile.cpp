#include "GeompackFile.hpp"

#include <iostream>
#include <fstream>

using namespace std;

#include "../../ece297/XMLWriter/XMLWriter.hpp"
#include "../../ece297/OSMReader/repeat_iterator.hpp"

#include <boost/range.hpp>
#include <boost/bind.hpp>

template<class PointRange,class Sizes,class CellRange>void writeXML_cells(string fn,PointRange P,Sizes S,CellRange C);

namespace boost {
	template<>struct range_difference<GeompackMesh::Tetra>{
		typedef unsigned long type;
	};
	template<>range_difference<GeompackMesh::Tetra>::type size<GeompackMesh::Tetra>(const GeompackMesh::Tetra&){ return 4; }
}

int main(int argc,char **argv)
{
	if (argc != 2)
	{
		cerr << "Needs exactly one argument: file name of Geompack++ file" << endl;
		return -1;
	}
	string fn(argv[1]);

	cout << "Loading file " << fn << endl;

	GeompackMesh GM(fn,true);

	writeXML_cells("geompack_out.xml",
			GM.getVTKTetraPoints(),
			make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>),
			make_transform_adaptor(
				make_iiterator_adaptor(GM.getVTKTetraCells(),GeompackMesh::GetVertices),
				std::bind<unsigned>(std::minus<unsigned>(),std::placeholders::_1,1U)));

	cout << "First tetra vertices are: ";

	//copy(boost::begin(t),boost::end(t),ostream_iterator<unsigned>(cout," "));
	//cout << endl;

	cout << "There are " << boost::distance(make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>)) << " offset-size entries" << endl;
	cout << "There are " << boost::distance(make_iiterator_adaptor(GM.getVTKTetraCells(),GeompackMesh::GetVertices)) << " connectivity entries" << endl;

	unsigned i=0;

	vector<unsigned> foobar(10);
	unsigned s = boost::size((const vector<unsigned>&)foobar);

	cout << "boost::size(ten-element vector): " << s << endl;

	cout << "first 20 vertices: " << endl;

	for(const GeompackMesh::Tetra& t : GM.getVTKTetraCells())
		if (i++ == 20)
			break;
		else
			cout << t.v << endl;

	i=0;
	for(const unsigned& v : make_iiterator_adaptor(GM.getVTKTetraCells(),boost::mem_fn(&GeompackMesh::Tetra::getVertices)))
		if (i++ == 20)
			break;
		else
			cout << v << endl;

	i=0;
	for(size_t s : make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>))
			if (i++ == 20)
				break;
			else
				cout << s << endl;


	cout << "Lazy scan explicit form: " << endl;
	i=0;
	typedef unsigned cell_difference;
	auto ls = make_lazy_scan<cell_difference>(std::plus<cell_difference>());
	for(const cell_difference& s : make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>))
				if (i++ == 20)
					break;
				else
					cout << ls(s) << endl;

	cout << "Lazy scan transformer: " << endl;
	i=0;
	for(const cell_difference& s : make_transform_adaptor(
			make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>),
			make_lazy_scan<unsigned>(std::plus<unsigned>())))
		if (i++ == 20)
			break;
		else
			cout << s << endl;

	/*

	xml_WriteDataArray(cout,"lazy_scan_transformer",make_transform_adaptor(
			make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>),
			make_lazy_scan<unsigned long>(std::plus<unsigned long>())));
	*/

/*	auto r = make_transform_adaptor(GM.getVTKTetraCells(),boost::size<GeompackMesh::Tetra>);

	for(cell_difference cd : make_transform_adaptor(
										r,
										make_lazy_scan<cell_difference>(std::plus<cell_difference>())))
		cout << cd << endl;*/


	return 0;
}

/** Writes out a complete VTK file.
 * Cell types: polyline (4), tetra (10)
 */

template<class PointRange,class Sizes,class CellRange>void writeXML_cells(string fn,PointRange P,Sizes S,CellRange C,unsigned cell_type=10)
	{
		unsigned long Nc = boost::distance(S);
		typedef unsigned cell_difference;
		ofstream os(fn.c_str());
		{
			XMLNode n(os,"VTKFile",
					make_pair("type","UnstructuredGrid"));
			{
				XMLNode n(os,"UnstructuredGrid");
				{
					XMLNode n(os,"Piece",
							make_pair("NumberOfPoints",boost::size(P)),
							make_pair("NumberOfCells",Nc));
					{
						XMLNode n(os,"Points");
						xml_WriteDataArray(os,"",boost::begin(P),
								boost::end(P));
					}
					{
						XMLNode n(os,"Cells");
						xml_WriteDataArray(os,"connectivity",boost::begin(C),boost::end(C));

						xml_WriteDataArray(os,"offsets",
								make_transform_adaptor(
									S,
									make_lazy_scan<cell_difference>(std::plus<cell_difference>())));

						for(unsigned long t : S)
							cout << t << endl;

						xml_WriteDataArray(os,"types",make_repeat_adaptor<unsigned char>(cell_type,Nc));
					}
					{
						XMLNode n(os,"CellData",make_pair("Scalars","Region"));
						xml_WriteDataArray(os,"Region",make_repeat_adaptor<float>(1.0,Nc));
					}
				}
			}
		}
	}
