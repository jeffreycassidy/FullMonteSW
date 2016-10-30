package require vtk

load libFullMonteVTKTCL.so
load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so

TIMOSAntlrParser R
    R setMeshFileName "/Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse.mesh"

set mesh [R mesh]

wm withdraw .

Ball B
    B centre "1.0 2.0 3.0"
    B radius 10.0

Point P
    P position "2.0 2.0 3.0"

Line L
    L endpoint 0 "0.0 0.0 0.0"
    L endpoint 1 "2.0 4.0 6.0"

Volume V 1.0 1234

SurfaceTri ST 1.0 "1 2 3"

Surface SF 1.0 5000

set C [Composite]
    $C power 1.0
    $C add B
    $C add P
    $C add V
    $C add L
    $C add ST
    $C add SF

vtkSourceExporter S
    S source $C
    S mesh $mesh

vtkUnstructuredGridWriter W
    W SetFileName "Sources.vtk"
    W SetInputData [S output]
    W Update
    W Delete
