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
#include <vtkScalarBarActor.h>

#include <vtkConeSource.h>
#include <vtkGlyph3D.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>

#include <vtkCellData.h>

#include <vtkLookupTable.h>

#include <vtkTubeFilter.h>

#include <vtkPlane.h>
#include <vtkImplicitPlaneRepresentation.h>
#include <vtkImplicitPlaneWidget2.h>
#include <vtkCallbackCommand.h>
#include <vtkClipPolyData.h>

#include <vtkClipDataSet.h>

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
	void AddActor2D(vtkSmartPointer<vtkActor2D> a){ ren->AddActor2D(a); }
	void AddProp(vtkSmartPointer<vtkProp> p){ ren->AddActor(p); }
	void AddWidget(vtkSmartPointer<vtkInteractorObserver> w){ w->SetInteractor(iren); w->On(); }
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

pair<vtkSmartPointer<vtkPlane>,vtkSmartPointer<vtkImplicitPlaneRepresentation>> callbackdata;

unsigned char Const0(PointGraph::edge_descriptor){ return 0; }
bool AlwaysTrue(PointGraph::edge_descriptor){ return true; }

void callback_plane_widget_update(vtkObject*,unsigned long eid,void* clientdata,void *calldata)
{
	pair<vtkSmartPointer<vtkPlane>,vtkSmartPointer<vtkImplicitPlaneRepresentation>>& p =
			*(pair<vtkSmartPointer<vtkPlane>,vtkSmartPointer<vtkImplicitPlaneRepresentation>>*)clientdata;

	cout << "Plane widget was updated" << endl;
	p.second->GetPlane(p.first);
}

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
	for(unsigned i=80;i<95;++i)
		slices.push_back(i);

	vector<unsigned> rois;
	for(unsigned i=0;i<10;++i)
		rois.push_back(i);

	MeshGraph M(pf,slices,rois);

	M.writeAll("test");

	VisualizationWindow vw;

	// draw ribbon curves enclosing data
	vtkSmartPointer<vtkPolyData> ribbons = M.vtkRibbonFromCurves2(pf);

	vtkSmartPointer<vtkPolyDataMapper> ribbonMapper = vtkPolyDataMapper::New();
	ribbonMapper->SetInputData(ribbons);

		// display the scalars
		ribbonMapper->ScalarVisibilityOn();
		ribbonMapper->SetScalarModeToUseCellData();
		ribbonMapper->UseLookupTableScalarRangeOn();
		ribbonMapper->SetColorModeToMapScalars();

		vtkSmartPointer<vtkLookupTable> ribbonlut = vtkLookupTable::New();
		ribbonlut->IndexedLookupOn();
		ribbonlut->SetNumberOfTableValues(5);
		ribbonlut->Build();
		ribbonlut->SetNanColor(1.0,1.0,0.0,1.0);		// invalid colour yellow

		ribbonlut->SetTableValue(0,0,0,0,0);				// no connections: black, transparent
		ribbonlut->SetAnnotation(vtkVariant(0),"");

		ribbonlut->SetTableValue(1,1.0,0.0,0.0,1.0);		// singleton: red
		ribbonlut->SetAnnotation(vtkVariant(1),"");

		ribbonlut->SetTableValue(2,0.8,0.5,0.0,1.0);		// 2 neighbours: orange
		ribbonlut->SetAnnotation(vtkVariant(2),"");

		ribbonlut->SetTableValue(3,0.0,0.0,1.0,1.0);		// 3 neighbours (good): blue
		ribbonlut->SetAnnotation(vtkVariant(3),"");

		ribbonlut->SetTableValue(4,1.0,0.0,1.0,1.0);		// 4 neighbours (excessive): magenta
		ribbonlut->SetAnnotation(vtkVariant(4),"");

		ribbonMapper->SetLookupTable(ribbonlut);




	vtkSmartPointer<vtkActor> ribbonActor = vtkActor::New();
	ribbonActor->SetMapper(ribbonMapper);
	ribbonActor->GetProperty()->SetOpacity(0.75);
	//ribbonActor->GetProperty()->SetColor(0.0,0.0,1.0);

	cout << "Ribbons: " << ribbons->GetNumberOfCells() << " cells, " << ribbons->GetNumberOfPolys() << " points" << endl;



	// draw all of the graph edges
	//vtkSmartPointer<vtkPolyData> edgeData = M.filterEdges(std::bind1st(std::mem_fn(&MeshGraph::incident_boundary_faces),&M),Const0);
	//vtkSmartPointer<vtkPolyData> edgeData = M.filterEdges(AlwaysTrue,Const0);
	vtkSmartPointer<vtkPolyData> edgeData = M.filterEdges(std::bind1st(std::mem_fn(&MeshGraph::ExcludeZero),&M),Const0);
	//vtkSmartPointer<vtkPolyData> edgeData = M.filterEdges(std::bind1st(std::mem_fn(&MeshGraph::curve_edges),&M),Const0);
	vtkSmartPointer<vtkTubeFilter> edgeTubes = vtkTubeFilter::New();
	edgeTubes->SetInputData(edgeData);
	edgeTubes->SetRadius(0.005);
	edgeTubes->Update();

	vtkSmartPointer<vtkPolyDataMapper> edgeMapper = vtkPolyDataMapper::New();
	edgeMapper->ScalarVisibilityOff();
	edgeMapper->SetInputData(edgeTubes->GetOutput());

	vtkSmartPointer<vtkActor> edgeActor = vtkActor::New();
	edgeActor->SetMapper(edgeMapper);

	cout << "Edge dataset has " << edgeData->GetNumberOfCells() << " cells and " << edgeData->GetNumberOfPoints() << " points" << endl;


	// glyph the ribbon curve points to show point normals
	vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
		cone->SetResolution(6);
		cone->Update();

	vtkSmartPointer<vtkTransform> conetrans = vtkTransform::New();
		conetrans->Translate(0.5,0.0,0.0);

	vtkSmartPointer<vtkTransformPolyDataFilter> applytransform = vtkTransformPolyDataFilter::New();
		applytransform->SetTransform(conetrans);
		applytransform->SetInputData(cone->GetOutput());
		applytransform->Update();

	vtkSmartPointer<vtkGlyph3D> glyph = vtkGlyph3D::New();
		glyph->SetInputData(ribbons);
		glyph->SetSourceData(applytransform->GetOutput());
		glyph->SetVectorModeToUseVector();
		glyph->SetScaleModeToDataScalingOff();
		glyph->SetScaleFactor(0.05);
		glyph->Update();

	vtkSmartPointer<vtkPolyDataMapper> glyphMapper = vtkPolyDataMapper::New();
		glyphMapper->SetInputData(glyph->GetOutput());

	vtkSmartPointer<vtkActor> glyphActor = vtkActor::New();
		glyphActor->SetMapper(glyphMapper);




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


	// clipping plane
	vtkSmartPointer<vtkPlane> clipPlane = vtkPlane::New();

	// clipping plane visualization
	vtkSmartPointer<vtkImplicitPlaneRepresentation> planeRep = vtkImplicitPlaneRepresentation::New();
	planeRep->SetPlaceFactor(1.25);
	planeRep->OutlineTranslationOff();

	// now for the triangulation lines
	vtkSmartPointer<vtkPolyData> meshData = M.getVTKMeshLines();

	vtkSmartPointer<vtkClipPolyData> meshClipper = vtkClipPolyData::New();
	meshClipper->SetInputData(meshData);
	meshClipper->GenerateClippedOutputOff();
	meshClipper->SetClipFunction(clipPlane);
	meshClipper->SetValue(0.0);


	vtkSmartPointer<vtkPolyDataMapper> meshMapper = vtkPolyDataMapper::New();
	meshMapper->SetInputConnection(meshClipper->GetOutputPort());
	meshMapper->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> meshActor = vtkActor::New();
	meshActor->SetMapper(meshMapper);
	meshActor->GetProperty()->SetOpacity(1.0);
	meshActor->GetProperty()->SetColor(0.0,0.0,1.0);



	// now for the triangulation faces
	/*vtkSmartPointer<vtkPolyData> faceData = // ....

	vtkSmartPointer<vtkClipPolyData> faceClipper = vtkClipPolyData::New();
	faceClipper->SetInputData(faceData);
	faceClipper->GenerateClippedOutputOff();
	faceClipper->SetClipFunction(clipPlane);
	faceClipper->SetValue(0.0);


	vtkSmartPointer<vtkPolyDataMapper> faceMapper = vtkPolyDataMapper::New();
	faceMapper->SetInputConnection(faceClipper->GetOutputPort());
	faceMapper->ScalarVisibilityOff();

	vtkSmartPointer<vtkActor> faceActor = vtkActor::New();
	faceActor->SetMapper(faceMapper);
	faceActor->GetProperty()->SetOpacity(1.0);
	faceActor->GetProperty()->SetColor(0.0,0.0,1.0);*/



	// and the solid tetrahedra
	vtkSmartPointer<vtkUnstructuredGrid> tetData = M.getVTKTetraData_Types();

	vtkSmartPointer<vtkClipDataSet> tetClipper = vtkClipDataSet::New();
	tetClipper->SetInputData(tetData);
	tetClipper->GenerateClippedOutputOff();
	tetClipper->SetClipFunction(clipPlane);
	tetClipper->SetValue(0.0);

	vtkSmartPointer<vtkDataSetMapper> tetMapper = vtkDataSetMapper::New();
	tetMapper->SetInputConnection(tetClipper->GetOutputPort());
	tetMapper->ScalarVisibilityOn();
	tetMapper->SetColorModeToMapScalars();
	tetMapper->UseLookupTableScalarRangeOn();
	tetMapper->Update();
	tetMapper->SetScalarModeToUseCellFieldData();
	tetMapper->SelectColorArray("Tetra Types");

	unsigned Ncd=tetClipper->GetOutput()->GetCellData()->GetNumberOfArrays();
	cout << "  Scalar mode: " << tetMapper->GetScalarModeAsString() << endl;
	cout << "  Colour mode: " << tetMapper->GetColorModeAsString() << endl;

	cout << "Found " << Ncd << " CellData arrays" << endl;

	for(unsigned i=0;i<Ncd;++i)
		cout << "  Array " << i << ": " << tetClipper->GetOutput()->GetCellData()->GetArrayName(i) << endl;

	cout << "Colouring by array \"" << tetMapper->GetArrayName() << "\" (ID " << tetMapper->GetArrayId() << ")" << endl;

	vtkSmartPointer<vtkLookupTable> lut = vtkLookupTable::New();
	lut->IndexedLookupOn();
	lut->SetNumberOfTableValues(2);
	lut->Build();


	lut->SetTableValue(0,0.0,0.0,1.0,1.0);			// blue if unassigned
	lut->SetAnnotation(vtkVariant(0),"Unassigned tetra");

	lut->SetTableValue(1,1.0,0.0,0.0,1.0);		// red if outer
	lut->SetAnnotation(vtkVariant(1),"Boundary tetra");

	lut->SetNanColor(1.0,1.0,0.0,1.0);				// yellow for invalid
	tetMapper->SetLookupTable(lut);

	vtkSmartPointer<vtkActor> tetActor = vtkActor::New();
	tetActor->SetMapper(tetMapper);


	// actor to visualize curves
	vtkSmartPointer<vtkActor> curveActor = vtkActor::New();
	curveActor->SetMapper(curveMapper);
	curveActor->GetProperty()->SetOpacity(0.5);
	curveActor->GetProperty()->SetColor(1.0,1.0,1.0);



	callbackdata = make_pair(clipPlane,planeRep);

	//vw.AddActor(curveActor);
	vw.AddActor(edgeActor);
	//vw.AddActor(meshActor);
	//vw.AddActor(faceActor);
	//vw.AddActor(tetActor);

	planeRep->PlaceWidget(meshActor->GetBounds());


	// clipping plane widget
	vtkSmartPointer<vtkCallbackCommand> planeWidgetCallback = vtkCallbackCommand::New();
	planeWidgetCallback->SetCallback(callback_plane_widget_update);
	planeWidgetCallback->SetClientData(&callbackdata);

	vtkSmartPointer<vtkImplicitPlaneWidget2> planeWidget = vtkImplicitPlaneWidget2::New();
	planeWidget->SetRepresentation(planeRep);
	planeWidget->AddObserver(vtkCommand::InteractionEvent,planeWidgetCallback);
	//vw.AddProp(planeRep);
	//vw.AddWidget(planeWidget);

	vw.AddActor(ribbonActor);
	vw.AddActor(glyphActor);

//	vtkSmartPointer<vtkScalarBarActor> scale = vtkScalarBarActor::New();
//	scale->SetLookupTable(lut);
//	scale->SetOrientationToVertical();
//	scale->SetTitle("Title");

	vw.start();
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

	// int status;

	//cout << "Typeid of *begin(points_iiterator): " << abi::__cxa_demangle(typeid(*begin(points_iiterator)).name(),0,0,&status) << endl;

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

	//int status;
	//cout << "Typeid of *begin(points_iiterator): " << abi::__cxa_demangle(typeid(*begin(points_iiterator)).name(),0,0,&status) << endl;

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

