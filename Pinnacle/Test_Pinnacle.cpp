#include "PinnacleFile.hpp"

#include "PinnacleVTK.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

#include "../../ece297/XMLWriter/XMLWriter.hpp"

#include <boost/iterator/counting_iterator.hpp>

#include <vtkDataSetMapper.h>
#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataMapper.h>

#include <vtkTubeFilter.h>

#include <thread>

class VisualizationWindow {
	thread t;
	vtkSmartPointer<vtkRenderer> ren;
	vtkSmartPointer<vtkRenderWindow> renwin;
	vtkSmartPointer<vtkRenderWindowInteractor> iren;
	vtkSmartPointer<vtkInteractorStyleSwitch> istyle;

	static void thread_start(VisualizationWindow*);

public:
	VisualizationWindow();

	void start();
	void waitUntilDone(){ t.join(); }

	void AddActor(vtkSmartPointer<vtkActor> a){ ren->AddActor(a); }
};

#include <vtkAutoInit.h>

#include "PinnacleGraph.hpp"

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
	pf.printSliceMap();

	auto p = pf.getCurves();

	unsigned i=0;

	ofstream os("points.txt",ios_base::out);

	auto pp = make_iiterator_adaptor(p,mem_fn(&Pinnacle::Curve::getPoints));
	for(const array<double,3> x : pp)
		os << x << endl;
	os.close();

	writeXML_Curves("curves.xml",pf);
	writeXML_Sets("sets.xml",pf);

	i=0;
	for(const Pinnacle::ROI& roi : pf.getROIs())
	{
		stringstream ss;
		ss << "roi." << i++ << ".txt";

		roi.exportCurves(ss.str());
	}

	map<double,unsigned> hist;

	auto curves_slice = make_iiterator_adaptor(pf.getROIs(),mem_fn(&Pinnacle::ROI::getCurves));
	for(const Pinnacle::Curve& c : curves_slice)
	{
		double z=c.getPoints().front()[2];
		//cout << "Curve of size " << c.size() << " z=" << z << endl;
		hist[z]++;
	}

//	for(const pair<double,unsigned>& p : hist)
//		cout << "  z=" << p.first << ": " << p.second << endl;

	// slice coordinates: z=82.5 has 4, z=85.9 has 30
	// 103.3: 37, 103.5: 33

	// graph rep

	vector<unsigned> slices;
	for(unsigned i=80;i<82;++i)
		slices.push_back(i);
	/*slices.push_back(83);
	slices.push_back(84);
	slices.push_back(85);
	slices.push_back(86);
	slices.push_back(87);
	slices.push_back(88);
	slices.push_back(89);
	slices.push_back(90);*/

	vector<unsigned> rois;
	for(unsigned i=0;i<10;++i)
		rois.push_back(i);

	MeshGraph M(pf,slices,rois);

	M.writeAll("test");

	VisualizationWindow vw;

	// the data
	vtkSmartPointer<vtkPolyData> curveData = M.getVTKCurveLines();

	// add a tube filter
	vtkSmartPointer<vtkTubeFilter> curveTubeFilter = vtkTubeFilter::New();
	curveTubeFilter->SetInputData(curveData);
	curveTubeFilter->SetRadius(0.04);
	curveTubeFilter->SetVaryRadiusToVaryRadiusOff();
	curveTubeFilter->SetNumberOfSides(5);
	curveTubeFilter->Update();
	curveTubeFilter->CappingOn();

	vtkSmartPointer<vtkPolyDataMapper> curveMapper = vtkPolyDataMapper::New();
	curveMapper->SetInputData(curveTubeFilter->GetOutput());
	curveMapper->ScalarVisibilityOff();
	curveMapper->Update();




	// now for the triangulation lines
	vtkSmartPointer<vtkPolyData> meshData = M.getVTKMeshLines();

	vtkSmartPointer<vtkPolyDataMapper> meshMapper = vtkPolyDataMapper::New();
	meshMapper->SetInputData(meshData);
	meshMapper->ScalarVisibilityOff();
	meshMapper->Update();

	vtkSmartPointer<vtkActor> meshActor = vtkActor::New();
	meshActor->SetMapper(meshMapper);
	meshActor->GetProperty()->SetOpacity(1.0);
	meshActor->GetProperty()->SetColor(0.0,0.0,1.0);



	// and the solid tetrahedra
	vtkSmartPointer<vtkUnstructuredGrid> tetData = M.getVTKMeshTetras();

	vtkSmartPointer<vtkDataSetMapper> tetMapper = vtkDataSetMapper::New();
	tetMapper->SetInputData(tetData);
	tetMapper->ScalarVisibilityOff();
	tetMapper->Update();

	vtkSmartPointer<vtkActor> tetActor = vtkActor::New();
	tetActor->SetMapper(tetMapper);
	tetActor->GetProperty()->SetOpacity(0.5);
	tetActor->GetProperty()->SetColor(0.0,1.0,0.0);
	tetActor->GetProperty()->SetRepresentationToSurface();


	// actor to visualize curves
	vtkSmartPointer<vtkActor> curveActor = vtkActor::New();
	curveActor->SetMapper(curveMapper);
	curveActor->GetProperty()->SetOpacity(1.0);
	curveActor->GetProperty()->SetColor(1.0,1.0,1.0);

	vw.AddActor(curveActor);
	vw.AddActor(meshActor);
	vw.AddActor(tetActor);
	vw.start();
	//cout << "Hello" << endl;
	//vw.waitUntilDone();
}


VisualizationWindow::VisualizationWindow()
{
	ren = vtkRenderer::New();

	renwin = vtkRenderWindow::New();
	renwin->AddRenderer(ren);

	iren = vtkRenderWindowInteractor::New();
	iren->SetRenderWindow(renwin);

	istyle = vtkInteractorStyleSwitch::New();
	iren->SetInteractorStyle(istyle);
}

void VisualizationWindow::thread_start(VisualizationWindow* vw_)
{
	if (vw_->iren)
	{
		vw_->renwin->Render();
		vw_->iren->Start();
	}
	else
		cerr << "Error: trying to start a NULL rendering window interactor in VisualizationWindow" << endl;
}

void VisualizationWindow::start()
{
	//t = thread(thread_start,this);
	thread_start(this);
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
							for(unsigned i=0; i<r.getprop<unsigned>("num_curve"); ++i)
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

