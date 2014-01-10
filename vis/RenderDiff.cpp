#include <vtkDataSetReader.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkCellData.h>
#include <vtkCellArray.h>
#include <vtkDataSetToPolyDataFilter.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>

#include <boost/smart_ptr/shared_ptr.hpp>

#include "../fm-postgres/fm-postgres.hpp"
#include "../fm-postgres/fmdbexportcase.hpp"
#include "fm_vtk.hpp"

#include "../blob.hpp"
#include "../histogram.hpp"
#include "../fluencemap.hpp"
#include "../newgeom.hpp"
#include "../graph.hpp"
#include "../logger.hpp"

#include <list>
#include <string>

//#include <sys/types.h>
#include <sys/stat.h>
#include <boost/smart_ptr/scoped_ptr.hpp>

using namespace std;

#include <boost/program_options.hpp>
#include <boost/program_options/errors.hpp>

namespace po=boost::program_options;
bool readFileBin(string fn,vector<Point<3,double> >& P,map<FaceByPointID,double>& m);

namespace globalopts {
    string title="Legend";
    vector<string> fn;
    vector<unsigned> mesh;
    string meshfn,vtkfn,binfile;
    string percentile("2.5-97.5");
    double ub=10000,lb=1;
    bool logplot=true;
    string range;
    vector<unsigned> runs;
};

FluenceMapBase* exportResultSet(PGConnection* conn,unsigned IDr,unsigned dType,const TetraMesh* mesh=NULL)
{
    unsigned long long packets;
    FluenceMapBase* data;

    Oid data_oid;

    PGConnection::ResultType res = conn->execParams("SELECT data_oid,launch FROM resultdata JOIN runresults ON runresults.runid=resultdata.runid WHERE resultdata.runid=$1 AND datatype=$2;",
        boost::tuples::make_tuple(IDr,dType));
    unpackSinglePGRow(res,boost::tuples::tie(data_oid,packets));

//    cout << "Run " << globalopts::runs[0] << ": " << packets_a << " launched" << endl;

    switch(dType){
        case 1: data = new SurfaceFluenceMap(mesh); break;
        case 2: data = new VolumeFluenceMap(mesh);  break;
        default: throw string("Error in exportResultSet: invalid datatype");
    }

    Blob b = conn->loadLargeObject(data_oid);
    data->fromBinary(b,packets);

    return data;
}

void renderScene(const list<vtkSmartPointer<vtkProp> >& props);
vtkSmartPointer<vtkActor> renderSurface(const TetraMesh& mesh);
vtkSmartPointer<vtkActor> renderFluence(const TetraMesh& mesh,SurfaceFluenceMap& surf,bool logScale,pair<double,double> range,bool=false);
vtkSmartPointer<vtkScalarBarActor> makeScaleBar(string title,vtkSmartPointer<vtkPolyDataMapper> mapper);

int main(int argc,char **argv)
{
    TetraMesh *m=NULL;

    po::options_description cmdline("Command-line options");

    cmdline.add_options()
        ("help,h","Display option help")
        ("run,R",po::value<vector<unsigned> >(&globalopts::runs),"Specify which run(s) to display")
        ("abs,a","Display display absolute value of relative error")
        ("gray,g","Plot grayscale")
        ("mesh,m",po::value<vector<unsigned> >(&globalopts::mesh),"Mesh specifier")
        ("file,f",po::value<vector<string> >(&globalopts::fn),"Surface fluence map file specifier")
//        ("input,i",po::value<string>(&globalopts::fn),"Input files")
//        ("binary,b",po::value<string>(&globalopts::binfile),"Binary input file with points & fluence")
//        ("mesh,m",po::value<string>(&globalopts::meshfn),"Mesh file")
//        ("vtkfile,v",po::value<string>(&globalopts::vtkfn),"VTK input file")
//        ("title,t",po::value<string>(&globalopts::title),"Legend title")
        ("percentile,p",po::value<string>(&globalopts::percentile),"Percentiles to display (either single number centered, or a range)")
        ("log,l","Logarithmic scale")
        ("range,r",po::value<string>(&globalopts::range),"Range of data to display")
        ;

    cmdline.add(globalopts::db::dbopts);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc,argv).options(cmdline).run(),vm);
        po::store(po::parse_environment(globalopts::db::dbopts,globalopts::db::dbEnvironmentMap),vm);
        po::notify(vm);
    }
    catch (po::error& e)
    {
        cerr << "Caught an exception in options processing" << endl;
        cout << cmdline << endl;
        return -1;
    }

    if (vm.count("help"))
    {
        cout << cmdline << endl;
        return -1;
    }

    if (vm.count("range"))
    {
        char c;
        stringstream ss(globalopts::range.c_str());
        ss >> globalopts::lb >> c >> globalopts::ub;
        cout << "INFO: Range set to [" << globalopts::lb << "," << globalopts::ub << "]" << endl;
    }

    double p,pl=0.025,pu=0.975;

    if (vm.count("percentile"))
    {
        stringstream ss(globalopts::percentile);
        ss >> p;
        if (ss.peek()=='-' || ss.peek()==',') // expressed as range, eg. 2.5-97.5
        {
            ss.ignore(1);
            pl = p;
            ss >> pu;
        }
        else // expressed as centered percentage about 50 (90 => 5,95)
        {
            pl = 50.0-p/2.0;
            pu = 50.0+p/2.0;
        }
        cout << "INFO: Range specified by percentile " << pl << "-" << pu << endl;
        pl *= 0.01;
        pu *= 0.01;
    }

    SurfaceFluenceMap *surf_a=NULL,*surf_b=NULL;

    boost::shared_ptr<PGConnection> dbconn = PGConnect();

    unsigned runid;

    unsigned IDm=1;

    if (globalopts::runs.size()>0)
    {
        runid=globalopts::runs.front();
        unsigned datatype=1;                // only renders surfaces for now

        m = exportMesh(*(dbconn.get()),IDm);
    
        cout << "Retrieving result set " << runid << endl;
    
        try {
            // load data set a

            surf_a = static_cast<SurfaceFluenceMap*>(exportResultSet(dbconn.get(),globalopts::runs[0],datatype,m));
    
            if (globalopts::runs.size() == 2)
            {
                globalopts::logplot=false;
                cout << "Displaying difference" << endl;

                surf_b = static_cast<SurfaceFluenceMap*>(exportResultSet(dbconn.get(),globalopts::runs[1],datatype,m));

//                cout << "Run " << globalopts::runs[1] << ": " << packets_b << " launched" << endl;
//    			cout << "Launched packets A=" << packets_a << " B=" << packets_b << " ratio=" << pkt_ratio << endl;

                *surf_b *= 1e9;
    
                *surf_a /= *surf_b;
    
                if (vm.count("abs"))
                    surf_a->absdiff();
            }
        }
        catch(PGConnection::PGConnectionException& e)
        {
            cerr << "Database failure with message " << e.msg << endl;
        }
    }
    else if (globalopts::fn.size()>0)
    {
        string fn=globalopts::fn.front();
        unsigned packets=0;
        size_t p;
        if ((p = fn.find_first_of(':')) != string::npos)
        {
            packets=stoull(fn.substr(0,p));
            fn = fn.substr(p+1);
        }

        cout << "Loading surface fluence map from " << fn << endl;
        cout << "  with " << packets << " packets launched" << endl;
        surf_a = new SurfaceFluenceMap(m);
        surf_a->loadASCII(fn,packets);
        if (globalopts::mesh.size() > 0)
        {
            cout << "Loading mesh ID=" << globalopts::mesh.front() << endl;
            m = exportMesh(*(dbconn.get()),globalopts::mesh.front());
            surf_a->setMesh(m);
        }
        else {
            cerr << "Error: must specify a mesh ID to use for plotting" << endl;
            return -1;
        }
    }
    else {
        cerr << "No runs specified" << endl;
        return -1;
    }

    Histogram hist(surf_a->valuesBegin(),surf_a->valuesEnd(),surf_a->size());

    pair<double,double> range;

    if (vm.count("range"))
        range = make_pair(globalopts::lb,globalopts::ub);
    else
        range = hist.getRange(pl,pu);

    cout << "Fluence range [" << hist.getMin() << ',' << hist.getMax() << "]" << endl;
    cout << "Display range [" << range.first << ',' << range.second << "]" << endl;

//    cout << "Total fluence " << surf_a->getTotalEnergy() << endl;

    list<vtkSmartPointer<vtkProp> > props;

    props.push_back(renderSurface(*m));

    vtkSmartPointer<vtkActor> fluenceActor = renderFluence(*m,*surf_a,globalopts::logplot,range,vm.count("gray"));
    props.push_back(fluenceActor);
    vtkSmartPointer<vtkProp> scaleBar = makeScaleBar("Scale",dynamic_cast<vtkPolyDataMapper*>(fluenceActor->GetMapper()));
    props.push_back(scaleBar);

    renderScene(props);
}

vtkSmartPointer<vtkActor> renderSurface(const TetraMesh& mesh)
{
    vtkSmartPointer<vtkPolyDataMapper> surfMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    vtkPoints       *points = vtkPoints::New();
    vtkCellArray    *celldata = vtkCellArray::New();
    vtkPolyData     *surface = vtkPolyData::New();

    for_each(mesh.pointBegin(),mesh.pointEnd(),VTKPointInsertNext(points));
    for_each(mesh.faceIDBegin(),mesh.faceIDEnd(),VTKCellInsertNext(celldata));

    // set up the new surface
    surface->SetPoints(points);
    points->Delete();
    surface->SetPolys(celldata);
    celldata->Delete();
    surface->Update();

    // set up the mapper
    surfMapper->SetInput(surface);

    vtkSmartPointer<vtkActor> surf1 = vtkSmartPointer<vtkActor>::New();
    surf1->SetMapper(surfMapper);
    surf1->GetProperty()->SetAmbient(0.25);
    surf1->GetProperty()->SetDiffuse(0.0);
    surf1->GetProperty()->SetSpecular(0.5);

    return surf1;
}

vtkSmartPointer<vtkActor> renderFluence(const TetraMesh& mesh,SurfaceFluenceMap& surf,bool logScale,pair<double,double> range,bool gray)
{
    vtkSmartPointer<vtkPolyDataMapper> surfMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    vtkPoints *points = vtkPoints::New();
    vtkCellArray *celldata = vtkCellArray::New();
    vtkFloatArray *scalars = vtkFloatArray::New();
    vtkPolyData *surface = vtkPolyData::New();

    for_each(mesh.pointBegin(),mesh.pointEnd(),VTKPointInsertNext(points));
    for_each(surf.fluenceByIDpBegin(),surf.fluenceByIDpEnd(),VTKCellInsertNext(celldata,scalars));

    // set up the new surface
    surface->SetPoints(points);
    points->Delete();
    surface->SetPolys(celldata);
    celldata->Delete();
    surface->GetCellData()->SetScalars(scalars);
    scalars->Delete();
    surface->Update();

    // set up the mapper
    surfMapper->SetInput(surface);
    surfMapper->SetScalarRange(range.first,range.second);
    surfMapper->ScalarVisibilityOn();

    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

    if (gray)
    {
        lut->SetSaturationRange(0.0,0.0);
        lut->SetHueRange(0.0,0.0);
        lut->SetValueRange(0.0,1.0);
    }

    if (logScale)
        lut->SetScaleToLog10();

    surfMapper->SetLookupTable(lut);

    vtkSmartPointer<vtkActor> surf1 = vtkSmartPointer<vtkActor>::New();
    surf1->SetMapper(surfMapper);
    surf1->GetProperty()->SetAmbient(0.5);
    surf1->GetProperty()->SetDiffuse(0.0);
    surf1->GetProperty()->SetSpecular(0.0);

    return surf1;
}

vtkSmartPointer<vtkScalarBarActor> makeScaleBar(string title,vtkSmartPointer<vtkPolyDataMapper> mapper)
{
    // Add a scale bar
    vtkSmartPointer<vtkScalarBarActor> bar1 = vtkSmartPointer<vtkScalarBarActor>::New();
    bar1->SetLookupTable(mapper->GetLookupTable());
    bar1->SetTitle(title.c_str());
    bar1->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    bar1->GetPositionCoordinate()->SetValue(0.1,0.01);
    bar1->SetOrientationToHorizontal();
    bar1->SetWidth(0.8);
    bar1->SetHeight(0.17);

    return bar1;
}


void renderScene(const list<vtkSmartPointer<vtkProp> >& props)
{
    vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
    renWin->AddRenderer(ren1);
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    iren->SetRenderWindow(renWin);
  
    // Add the props to the renderer, set the background and size.
    for(list<vtkSmartPointer<vtkProp> >::const_iterator it=props.begin(); it != props.end(); ++it)
        ren1->AddActor(*it);

    ren1->SetBackground(0.0, 0.0, 0.0);
    renWin->SetSize(800, 600);
  
    // Set up the lighting.
    vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
    light->SetFocalPoint(1.875,0.6125,0);
    light->SetPosition(0.875,1.6125,1);
    ren1->AddLight(light);
  
    // We want to eliminate perspective effects on the apparent lighting.
    // Parallel camera projection will be used. To zoom in parallel projection
    // mode, the ParallelScale is set.

    ren1->GetActiveCamera()->SetFocalPoint(0,0,0);
    ren1->GetActiveCamera()->SetPosition(0,0,100);
    ren1->GetActiveCamera()->SetViewUp(0,1,0);
    ren1->GetActiveCamera()->ParallelProjectionOn();
    ren1->ResetCamera();
    ren1->GetActiveCamera()->SetParallelScale(50);

    // This starts the event loop and invokes an initial render.
    iren->Initialize();
    iren->Start();
}
