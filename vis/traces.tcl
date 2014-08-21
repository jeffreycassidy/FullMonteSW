package require vtk

vtkXMLPolyDataReader reader
reader SetFileName "output_stripped.xml"

vtkWindowedSincPolyDataFilter smoother
smoother SetInputConnection [reader GetOutputPort]
smoother SetNumberOfIterations 50

vtkPolyDataNormals normals
normals SetInputConnection [smoother GetOutputPort]

# create a mapper/actor to display the plain surface
vtkPolyDataMapper surfacemapper
surfacemapper SetInputConnection [normals GetOutputPort]
surfacemapper ScalarVisibilityOff

vtkActor surfaceactor
surfaceactor SetMapper surfacemapper
[surfaceactor GetProperty] SetOpacity 0.1



vtkXMLUnstructuredGridReader tracereader
tracereader SetFileName "traces.xml"

tracereader Update

puts "traces.xml contains [tracereader GetNumberOfCells] cells"

vtkDataSetMapper tracermapper
    tracermapper SetInputConnection [tracereader GetOutputPort]
    tracermapper ScalarVisibilityOff

vtkActor traceractor
    traceractor SetMapper tracermapper
    [traceractor GetProperty] SetOpacity 0.2
    [traceractor GetProperty] SetColor 1 1 1

vtkRenderer ren
ren AddActor surfaceactor
ren AddActor traceractor
vtkRenderWindow renwin
renwin AddRenderer ren

wm withdraw .

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render
iren Start


