package require vtk

#set fn "sets.xml"

#vtkXMLUnstructuredGridReader reader
#reader SetFileName $fn

#reader Update

#puts "$fn contains [reader GetNumberOfCells] cells"
#puts "$fn contains [reader GetNumberOfPoints] points"

# get some tetra data in here

load TetraMeshTCL.so

#set conn [tclConnect]

#set mesh [loadMesh $conn 1]
set mesh [loadMeshFile "data/mouse.mesh"]

set td [getVTKTetraMesh $mesh]

set phi_v [loadVectorDouble "DBUtils/out.3093.phi_v.txt"]

set fluence [getVTKDataArray $phi_v]

[$td GetCellData] SetScalars $fluence

vtkPlane plane

vtkImplicitPlaneRepresentation planerep
    planerep SetPlaceFactor 1.25

vtkClipDataSet clipper
    clipper SetInputData $td
    clipper SetClipFunction plane

vtkDataSetMapper mapper
    mapper SetInputConnection [clipper GetOutputPort]
#    mapper SetInputData $td

#vtkLookupTable lut

#lut SetNumberOfTableValues 52

#for { set i 0 } { $i < 52 } { incr i } { lut SetTableValue $i 1 1 1 1 }

#mapper SetLookupTable lut

vtkActor actor
actor SetMapper mapper

vtkRenderer ren
ren AddActor actor
ren AddActor planerep

vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin

proc PlaneUpdate {} {
    puts "Updating clipping plane"
    planerep GetPlane plane
    clipper Update
}

renwin Render

vtkImplicitPlaneWidget2 planewidget
    planewidget SetInteractor iren
    planewidget SetRepresentation planerep
    planewidget AddObserver EndInteractionEvent PlaneUpdate

planerep OutlineTranslationOff

planerep PlaceWidget 0 60 0 60 0 20

planewidget On

renwin Render
