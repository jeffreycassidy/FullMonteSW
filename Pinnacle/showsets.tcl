package require vtk

set fn "sets.xml"

vtkXMLUnstructuredGridReader reader
reader SetFileName $fn

reader Update

puts "$fn contains [reader GetNumberOfCells] cells"
puts "$fn contains [reader GetNumberOfPoints] points"

vtkPlane plane

vtkImplicitPlaneRepresentation planerep
    planerep SetPlaceFactor 1.25

vtkClipDataSet clipper
    clipper SetInputConnection [reader GetOutputPort]
    clipper SetClipFunction plane

vtkDataSetMapper mapper
    mapper SetInputConnection [clipper GetOutputPort]

vtkLookupTable lut

lut SetNumberOfTableValues 52

for { set i 0 } { $i < 52 } { incr i } { lut SetTableValue $i 1 1 1 1 }

#lut SetTableValue 51 1 1 1 0.5
#lut SetTableValue 50 1 0 0 0.5
#lut SetTableValue 49 0 1 0 0.5
#lut SetTableValue 48 0 0 1 0.5
#lut SetTableValue 47 0 1 1 0.5
#lut SetTableValue 46 1 0 1 0.5
#lut SetTableValue 45 1 1 0 0.5

mapper SetLookupTable lut

#vtkDelaunay3D delaunay
#    delaunay SetInputConnection [reader GetOutputPort]

#vtkDataSetMapper delaunaymapper
#    delaunaymapper SetInputConnection [delaunay GetOutputPort]

#vtkLODActor delaunayactor
#    delaunayactor SetMapper delaunaymapper
#    [delaunayactor GetProperty] SetOpacity 0.1

vtkRenderer ren
ren AddActor actor
ren AddActor planerep

#ren AddActor delaunayactor
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin

proc PlaneUpdate {} {
    puts "Updating clipping plane"
    planerep GetPlane plane
}

vtkImplicitPlaneWidget2 planewidget
    planewidget SetInteractor iren
    planewidget SetRepresentation planerep
    planewidget AddObserver InteractionEvent PlaneUpdate

planerep OutlineTranslationOff

planerep PlaceWidget -30 30 -30 30 60 120
#planerep SetNormal [plane GetNormal]

planewidget On


renwin Render

iren Start


