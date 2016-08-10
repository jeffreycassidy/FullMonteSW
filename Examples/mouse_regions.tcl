## Loads the Digimouse mesh (provided with TIM-OS) and converts it to a .vtk file showing the regions

package require vtk

load libFullMonteGeometryTCL.so
load libFullMonteTIMOSTCL.so
load libFullMonteVTKTCL.dylib

set meshfn "/Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse.mesh"

TIMOSAntlrParser R
R setMeshFileName $meshfn

set mesh [R mesh]

vtkFullMonteTetraMeshWrapper vtkM
    vtkM mesh $mesh

vtkUnstructuredGridWriter W
    W SetFileName "mouse.regions.vtk"
    W SetInputData [vtkM regionMesh]
    W Update
    W Delete
