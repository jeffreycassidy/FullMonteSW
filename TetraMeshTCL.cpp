#include "TetraMeshTCL.i"
#include <vtk/vtkTclUtil.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <string>
#include "VTKInterface.hpp"
#include "graph.hpp"

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
	globalopts::db::blobCachePath = "/home/jcassidy/fullmonte/blobcache";
	// Normally set by environment parsed by boost::program_options

	conn=PGConnect();
	return conn.get();		// Careful! Managed by boost shared_ptr; need to keep it global to avoid premature destruction
}

/*TetraMesh* loadMesh(PGConnection* conn,unsigned IDm)
{
	return exportMesh(*conn,IDm);
}*/

TetraMesh* loadMesh(const std::string& fn)
{
	TetraMesh* M=new TetraMesh(fn,TetraMesh::MatlabTP);
	return M;
}
/*TetraMesh* loadMesh(const char* fn)
{
	TetraMesh* M=new TetraMesh(fn,TetraMesh::MatlabTP);
	return M;
}*/

vtkPolyData* createVTKBoundary(const TetraMesh& M,unsigned matID)
{
	return getVTKPolyData(M.extractMaterialBoundary(matID));
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

vtkPolyData* getVTKRegion(const TetraMesh& M,const vector<unsigned>& tetIDs)
{
	TriSurf ts = M.extractRegionSurface(tetIDs);
	return getVTKPolyData(ts);
}
