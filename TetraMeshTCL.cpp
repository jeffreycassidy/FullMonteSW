#include "TetraMeshTCL.i"
#include <vtk/vtkTclUtil.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <string>
#include "VTKInterface.hpp"
#include "graph.hpp"
#include <vtkDoubleArray.h>
#include "Parallelepiped.hpp"

// Support code that has some constants needed for creation of VTK TCL references

template<class vtkObjectType>struct vtkObjectTraits;

#define VTK_OBJECT_TCL_WRAPPER(VTK_OBJECT_TYPE) \
	template<>struct vtkObjectTraits<VTK_OBJECT_TYPE>{ \
		static constexpr const char *typestr=#VTK_OBJECT_TYPE;\
		static vtkTclCommandStruct cs; \
	}; \
	ClientData VTK_OBJECT_TYPE##NewCommand(); \
	int VTK_OBJECT_TYPE##Command(ClientData cd, Tcl_Interp *interp,int argc,char **argv); \
	vtkTclCommandStruct vtkObjectTraits<VTK_OBJECT_TYPE>::cs = { VTK_OBJECT_TYPE##NewCommand, VTK_OBJECT_TYPE##Command };

VTK_OBJECT_TCL_WRAPPER(vtkUnstructuredGrid)
VTK_OBJECT_TCL_WRAPPER(vtkPolyData)


// not part of the exposed API - need to use with care
// This depends on VTK 6.1.0
vtkTclInterpStruct *vtkGetInterpStruct(Tcl_Interp *interp);

template<class vtkObjectType>vtkObjectType* createVTKTCLObject(Tcl_Interp* interp,const char* name)
{
	char *argv[2] = { (char*)vtkObjectTraits<vtkObjectType>::typestr, (char*)name };
	vtkTclNewInstanceCommand((ClientData*)&vtkObjectTraits<vtkObjectType>::cs,interp,2,argv);

	// look up the resulting object
	vtkTclInterpStruct *is = vtkGetInterpStruct(interp);

	if (Tcl_HashEntry *entry = Tcl_FindHashEntry(&is->InstanceLookup,name))
	{
		return static_cast<vtkObjectType*>(Tcl_GetHashValue(entry));
	}
	else {
		cerr << "Error creating object! " << endl;
		return NULL;
	}
}


boost::shared_ptr<PGConnection> conn;

extern "C" PGConnection* tclConnect()
{
	globalopts::db::blobCachePath = "/Users/jcassidy/fullmonte/blobcache";
	// Normally set by environment parsed by boost::program_options

	conn=PGConnect();
	return conn.get();		// Careful! Managed by boost shared_ptr; need to keep it global to avoid premature destruction
}

TetraMeshBase* loadMesh(PGConnection* conn,unsigned IDm)
{
	TetraMesh *M = exportMesh(*conn,IDm);
	return M;
}

TetraMeshBase* loadMeshFile(const std::string& fn)
{
	TetraMeshBase *tmb = new TetraMeshBase;
	tmb->readFileMatlabTP(fn);
	return tmb;
}

vtkPolyData* createVTKBoundary(const TetraMesh& M,unsigned matID)
{
	return getVTKPolyData(M.extractMaterialBoundary(matID));
}

vector<double> loadVectorDouble(const std::string& fn)
		{
	ifstream is(fn.c_str());
	vector<double> v;
	cout << "Reading a vector from " << fn << endl;
	copy(std::istream_iterator<double>(is),
			std::istream_iterator<double>(),
			std::back_inserter(v));
	for(double& d : v)
		d = log(d);
	cout << "  Read " << v.size() << " elements" << endl;
	return v;
		}

vector<unsigned> loadVector(const std::string& fn)
{
	ifstream is(fn.c_str());
	vector<unsigned> v;
	cout << "Reading a vector from " << fn << endl;
	copy(std::istream_iterator<unsigned>(is),
			std::istream_iterator<unsigned>(),
			std::back_inserter(v));
	cout << "  Read " << v.size() << " elements" << endl;
	return v;
}

vtkDataArray* getVTKDataArray(const vector<double>& v)
{
	return getVTKScalarArray<vtkDoubleArray>(v.begin(),v.end(),v.size());
}

vtkPolyData* getVTKRegion(const TetraMesh& M,const vector<unsigned>& tetIDs)
{
	TriSurf ts = M.extractRegionSurface(tetIDs);
	return getVTKPolyData(ts);
}

TetraMeshBase clipToRegion(const TetraMeshBase& M,const Parallelepiped& pp)
{
	return M.clipTo(pp);
}
