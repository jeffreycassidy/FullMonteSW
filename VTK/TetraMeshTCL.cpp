#include <vtk/vtkTclUtil.h>

// Support code that has some constants needed for creation of VTK TCL references

template<class vtkObjectType>struct vtkObjectTraits;

#define VTK_OBJECT_TCL_WRAPPER(VTK_OBJECT_TYPE) \
	class VTK_OBJECT_TYPE;								\
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
