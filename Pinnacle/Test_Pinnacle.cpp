#include "PinnacleFile.hpp"

#include <iostream>
#include <fstream>

#include "../../ece297/XMLWriter/XMLWriter.hpp"

#include <boost/iterator/counting_iterator.hpp>

template<class T,class Counter=unsigned long> class repeat_iterator {
	T v;
	Counter c;
public:

	typedef T value_type;
	typedef T& reference;
	typedef T* pointer;
	typedef Counter difference_type;

	repeat_iterator(const repeat_iterator&) = default;
	repeat_iterator(T v_,Counter c_=T()) : v(v_),c(c_){}

	T operator*() const { return v; }
	const T* operator->() const { return &v; }
	repeat_iterator& operator++(){ ++c; return *this; }

	bool operator==(const repeat_iterator& x) const { return x.c == c; }
	bool operator!=(const repeat_iterator& x) const { return x.c != c; }
};

template<class T,class Counter=unsigned long>repeat_iterator<T,Counter> make_repeat_iterator(T v_,Counter N_)
		{
	return repeat_iterator<T,Counter>(v_,N_);
		}

template<class T,class Counter=unsigned long>pair<repeat_iterator<T,Counter>,repeat_iterator<T,Counter>>
	make_repeat_adaptor(T v_,Counter N_)
{
	return make_pair(repeat_iterator<T,Counter>(v_,0),repeat_iterator<T,Counter>(v_,N_));
}


using namespace std;

void writeXML_Curves(string fn,const Pinnacle::File& F);
void writeXML_Sets(string fn,const Pinnacle::File& F);

int main(int argc,char **argv)
{
	if (argc < 2)
	{
		cerr << "Requires a file name specified at command line" << endl;
		return -1;
	}

	Pinnacle::File pf(argv[1]);

	pf.read();

	auto p = pf.getCurves();

	unsigned i=0;
	/*
	for(const Pinnacle::Curve& c : p)
	{
		cout << '[' << setw(3) << i++ << "] Curve with size " << c.size() << endl;
	}*/

	i=0;

	ofstream os("points.txt",ios_base::out);

	auto pp = make_iiterator_adaptor(p,mem_fn(&Pinnacle::Curve::getPoints));
	for(const array<double,3>& x : pp)
		os << x << endl;

	writeXML_Curves("curves.xml",pf);
	writeXML_Sets("sets.xml",pf);
}


void writeXML_Curves(string fn,const Pinnacle::File& F)
{
	auto points_iiterator = F.getPoints();
	auto curves_iiterator = F.getCurves();

	unsigned Ncurves=boost::distance(curves_iiterator);
	unsigned Npoints=boost::distance(points_iiterator);

	int status;

	cout << "Typeid of *begin(points_iiterator): " << abi::__cxa_demangle(typeid(*begin(points_iiterator)).name(),0,0,&status) << endl;

	ofstream os(fn.c_str());
	{
		XMLNode n(os,"VTKFile",
				make_pair("type","UnstructuredGrid"));
		{
			XMLNode n(os,"UnstructuredGrid");
			{
				XMLNode n(os,"Piece",
						make_pair("NumberOfPoints",Npoints),
						make_pair("NumberOfCells",Ncurves));

				{
					XMLNode n(os,"Points");
					xml_WriteDataArray(os,"",begin(points_iiterator),end(points_iiterator));
				}
				{
					XMLNode n(os,"Cells");
					xml_WriteDataArray(os,"connectivity",
							boost::make_counting_iterator(0U),boost::make_counting_iterator(Npoints));

					auto r = make_transform_adaptor(curves_iiterator,mem_fn(&Pinnacle::Curve::size));

					xml_WriteDataArray(os,"offsets",
							make_transform_adaptor(r,make_lazy_scan<unsigned>(std::plus<unsigned>())));

					xml_WriteDataArray(os,"types",make_repeat_adaptor<unsigned char>(7,Ncurves));
				}
				{
					XMLNode n (os,"CellData",
							make_pair("Scalars","ROI"));

					//xml_WriteDataArray(os,"ROI",make_iiterator_adaptor(F.getROIs(),mem_fn(&ROI::getIDRepeater())) );

					{
						XMLNode n (os,"DataArray",
								make_pair("Name","ROI"),
								make_pair("format","ascii"),
								make_pair("type","UInt8"));
						unsigned j=0;
						for(const Pinnacle::ROI& r : F.getROIs())
						{
							for(unsigned i=0;i<r.getprop<unsigned>("num_curve");++i)
								os << j << ' ';
							++j;
							os << endl;
						}
					}
				}
			}
		}
	}
}


void writeXML_Sets(string fn,const Pinnacle::File& F)
{
	auto points_iiterator = F.getPoints();

	//unsigned Ncurves=boost::distance(curves_iiterator);
	unsigned Npoints=boost::distance(points_iiterator);

	unsigned Nrois=F.getNRois();

	int status;

	cout << "Typeid of *begin(points_iiterator): " << abi::__cxa_demangle(typeid(*begin(points_iiterator)).name(),0,0,&status) << endl;

	ofstream os(fn.c_str());
	{
		XMLNode n(os,"VTKFile",
				make_pair("type","UnstructuredGrid"));
		{
			XMLNode n(os,"UnstructuredGrid");
			{
				XMLNode n(os,"Piece",
						make_pair("NumberOfPoints",Npoints),
						make_pair("NumberOfCells",Nrois));

				{
					XMLNode n(os,"Points");
					xml_WriteDataArray(os,"",begin(points_iiterator),end(points_iiterator));
				}
				{
					XMLNode n(os,"Cells");
					xml_WriteDataArray(os,"connectivity",
							boost::make_counting_iterator(0U),boost::make_counting_iterator(Npoints));

					auto r = make_transform_adaptor(F.getROIs(),mem_fn(&Pinnacle::ROI::getNPoints));
					xml_WriteDataArray(os,"offsets",
							make_transform_adaptor(r,make_lazy_scan<unsigned>(std::plus<unsigned>())));

					xml_WriteDataArray(os,"types",make_repeat_adaptor<unsigned char>(2,Nrois));
				}
				{
					XMLNode n (os,"CellData",
							make_pair("Scalars","ROI"));

					xml_WriteDataArray(os,"ROI",boost::make_counting_iterator(0U),boost::make_counting_iterator(Nrois));
				}
			}
		}
	}
}

