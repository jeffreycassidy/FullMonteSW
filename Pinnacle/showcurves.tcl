package require vtk

load PinnacleVTK.so

set fn plan_HN.roi
set roi 1

# read file
set pf [readPinnacleFile $fn]

# grab poly data for specified ROI
makeROIPolys $pf $roi polydata

# summarize

puts "$fn contains [polydata GetNumberOfCells] cells"
puts "$fn contains [polydata GetNumberOfPoints] points"

proc addroi { pf roi } {
    makeROIPolys $pf $roi polydata$roi

    vtkTriangleFilter tri$roi
        tri$roi SetInputData polydata$roi
        tri$roi Update

    vtkExtractEdges edges$roi
        edges$roi SetInputData polydata$roi

    vtkTubeFilter tube$roi
        tube$roi SetInputConnection [edges$roi GetOutputPort]
        tube$roi SetRadius 0.01
        tube$roi SetVaryRadiusToVaryRadiusOff
        tube$roi Update

    vtkPolyDataMapper tubemapper$roi
        tubemapper$roi SetInputConnection [tube$roi GetOutputPort]

    vtkActor tubeactor$roi
        tubeactor$roi SetMapper tubemapper$roi

    vtkPolyDataMapper mapper$roi
        mapper$roi SetInputConnection [tri$roi GetOutputPort]

    vtkActor actor$roi
        actor$roi SetMapper mapper$roi
        [actor$roi GetProperty] SetRepresentationToSurface
        [actor$roi GetProperty] SetOpacity 0.2

    ren AddActor actor$roi
    ren AddActor tubeactor$roi
}


vtkLookupTable lut

#lut SetNumberOfTableValues 52
#
#for { set i 0 } { $i < 52 } { incr i } { lut SetTableValue $i 0 0 0 0 }
#
#lut SetTableValue 51 1 1 1 0.5
#lut SetTableValue 50 1 0 0 0.5
#lut SetTableValue 49 0 1 0 0.5
#lut SetTableValue 48 0 0 1 0.5
#lut SetTableValue 47 0 1 1 0.5
#lut SetTableValue 46 1 0 1 0.5
#lut SetTableValue 45 1 1 0 0.5

#mapper SetLookupTable lut

vtkRenderer ren
vtkRenderWindow renwin
renwin AddRenderer ren

for { set i 0 } { $i < 15 } { incr i } { addroi $pf $i }

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render
iren Start

