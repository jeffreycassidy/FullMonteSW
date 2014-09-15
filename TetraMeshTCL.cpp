#include "fm-postgres/fm-postgres.hpp"
#include "VTKInterface.hpp"

#include "TetraMeshTCL.i"

#include <vtkRenderWindow.h>
#include <vtkActor.h>
#include <vtkRenderWindowInteractor.h>

#include <vtk/vtkTclUtil.h>

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

VTK_OBJECT_TCL_WRAPPER(vtkActor)
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

extern "C" TetraMesh* loadMesh(PGConnection* conn,unsigned IDm)
{
	return exportMesh(*conn,IDm);
}

extern "C" TriSurf* extractBoundary(const TetraMesh* M,unsigned IDmat)
{
	return new TriSurf(M->extractMaterialBoundary(IDmat));
}

void createVTKBoundary(Tcl_Interp* interp,const char *name,const TetraMesh* M, unsigned IDmat)
{
	vtkPolyData* pd = createVTKTCLObject<vtkPolyData>(interp,name);
	TriSurf	ts = M->extractMaterialBoundary(IDmat);
	getVTKPolyData(ts,pd);
}
