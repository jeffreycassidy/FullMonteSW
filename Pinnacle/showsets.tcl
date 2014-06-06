set fn "sets.xml"

vtkXMLUnstructuredGridReader reader
reader SetFileName $fn

reader Update

puts "$fn contains [reader GetNumberOfCells] cells"
puts "$fn contains [reader GetNumberOfPoints] points"

vtkDataSetMapper mapper
    mapper SetInputConnection [reader GetOutputPort]

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

vtkLODActor actor
    actor SetMapper mapper
#    [actor GetProperty] SetRepresentationToSurface
#    [actor GetProperty] SetOpacity 0.0
#    [actor GetProperty] SetColor 1 1 1

vtkRenderer ren
ren AddActor actor
#ren AddActor delaunayactor
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render
iren Start


