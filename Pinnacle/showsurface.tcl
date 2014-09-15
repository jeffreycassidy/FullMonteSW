package require vtk

set fn "sets.xml"

vtkXMLUnstructuredGridReader reader
reader SetFileName $fn

reader Update

puts "$fn contains [reader GetNumberOfCells] cells"
puts "$fn contains [reader GetNumberOfPoints] points"

vtkSurfaceReconstructionFilter surf
    surf SetInputConnection [reader GetOutputPort]

vtkContourFilter cf
    cf SetInputConnection [surf GetOutputPort]
    cf SetValue 0 0.0

vtkReverseSense rev
    rev SetInputConnection [cf GetOutputPort]
    rev ReverseCellsOn
    rev ReverseNormalsOn

vtkPolyDataMapper mapper
    mapper SetInputConnection [rev GetOutputPort]
    mapper ScalarVisibilityOff

vtkLODActor actor
    actor SetMapper mapper

vtkRenderer ren
ren AddActor actor
vtkRenderWindow renwin
renwin AddRenderer ren

renwin Render

iren Start


