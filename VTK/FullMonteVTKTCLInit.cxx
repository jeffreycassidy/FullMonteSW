#include "vtkTclUtil.h"
#include "vtkVersion.h"
#define VTK_TCL_TO_STRING(x) VTK_TCL_TO_STRING0(x)
#define VTK_TCL_TO_STRING0(x) #x

extern "C"
{
#if (TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4)
  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,int, CONST84 char *[]);
#else
  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,int, char *[]);
#endif
}

int TetraMeshVTK_TclCreate(Tcl_Interp *interp);
int SparseVectorVTK_TclCreate(Tcl_Interp *interp);
int PointSourceRep_TclCreate(Tcl_Interp *interp);

extern Tcl_HashTable vtkInstanceLookup;
extern Tcl_HashTable vtkPointerLookup;
extern Tcl_HashTable vtkCommandLookup;
extern void vtkTclDeleteObjectFromHash(void *);
extern void vtkTclListInstances(Tcl_Interp *interp, ClientData arg);


extern "C" {int VTK_EXPORT Fullmontevtktcl_SafeInit(Tcl_Interp *interp);}

extern "C" {int VTK_EXPORT Fullmontevtktcl_Init(Tcl_Interp *interp);}
extern "C" {int VTK_EXPORT Fullmontevtktcl_SafeUnload(Tcl_Interp *interp);}
extern "C" {int VTK_EXPORT Fullmontevtktcl_Unload(Tcl_Interp *interp);}

extern void vtkTclGenericDeleteObject(ClientData cd);


int VTK_EXPORT Fullmontevtktcl_SafeInit(Tcl_Interp *interp)
{
  return Fullmontevtktcl_Init(interp);
}


int VTK_EXPORT Fullmontevtktcl_Init(Tcl_Interp *interp)
{

  TetraMeshVTK_TclCreate(interp);
  SparseVectorVTK_TclCreate(interp);
  PointSourceRep_TclCreate(interp);

  char pkgName[]="FullMonteVTKTCL";
  char pkgVers[]=VTK_TCL_TO_STRING(VTK_MAJOR_VERSION) "." VTK_TCL_TO_STRING(VTK_MINOR_VERSION);
  Tcl_PkgProvide(interp, pkgName, pkgVers);
  return TCL_OK;
}

int VTK_EXPORT Fullmontevtktcl_Unload(Tcl_Interp *interp)
{
	return TCL_OK;

}

int VTK_EXPORT Fullmontevtktcl_SafeUnload(Tcl_Interp* interp)
{
	return Fullmontevtktcl_Unload(interp);

}
