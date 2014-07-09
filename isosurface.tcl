package require vtk

vtkXMLPolyDataReader surfreader
surfreader SetFileName "output_stripped.xml"

vtkWindowedSincPolyDataFilter smoother
smoother SetInputConnection [surfreader GetOutputPort]
smoother SetNumberOfIterations 50

vtkPolyDataNormals surfnormals
surfnormals SetInputConnection [smoother GetOutputPort]



# create a mapper/actor to display the plain surface

vtkXMLUnstructuredGridReader reader
reader SetFileName "volume_mesh.xml"

vtkDataSetMapper meshmapper
meshmapper SetInputConnection [reader GetOutputPort]
meshmapper ScalarVisibilityOn

#vtkDataSetMapper restmapper
#restmapper SetInputConnection [clipperInv GetOutputPort]
#restmapper ScalarVisibilityOff

vtkCellDataToPointData cell2point
cell2point SetInputConnection [reader GetOutputPort]
cell2point PassCellDataOn

vtkContourFilter contours
contours SetInputConnection [cell2point GetOutputPort]
contours SetNumberOfContours 4
contours SetValue 0 10
contours SetValue 1 100
contours SetValue 2 1000
contours SetValue 3 10000

vtkWindowedSincPolyDataFilter contoursmoother
contoursmoother SetInputConnection [contours GetOutputPort]
contoursmoother SetNumberOfIterations 50

vtkPolyDataNormals contournormals
contournormals SetInputConnection [contoursmoother GetOutputPort]

vtkPolyDataMapper contourmapper
contourmapper SetInputConnection [contournormals GetOutputPort]
contourmapper SetScalarRange 0.01 100
contourmapper ScalarVisibilityOff

vtkActor contouractor
contouractor SetMapper contourmapper
[contouractor GetProperty] SetColor 1 1 1
[contouractor GetProperty] SetOpacity 1

vtkPolyDataMapper surfmapper
surfmapper SetInputConnection [surfnormals GetOutputPort]
surfmapper ScalarVisibilityOff

vtkActor surfactor
surfactor SetMapper surfmapper
[surfactor GetProperty] SetOpacity 0.3
[surfactor GetProperty] SetColor 1 0.9 0.8

vtkActor meshactor
meshactor SetMapper meshmapper
[meshactor GetProperty] SetOpacity 0.3
[meshactor GetProperty] SetRepresentationToPoints

# create renderer and go
vtkRenderer ren
ren AddActor contouractor
ren AddActor surfactor
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render

iren Start


