package require vtk
load TetraMeshTCL.so

set IDr [lindex $argv 1]
set IDm [lindex $argv 0]

puts "dbvolview.tcl with mesh ID $IDm and run ID $IDr"

puts "Connecting to database"
set conn [tclConnect]

puts "Fetching mesh"
set mesh [loadMesh $conn $IDm]


puts "Fetching result data"
set fluence_vector [exportResultVector $conn $IDr 2]


# get the tetra mesh into td
set td [getVTKTetraMesh $mesh]

[$td GetCellData] SetScalars [getVTKDataArray $fluence_vector]

# surface smoothing and normal generation
#vtkWindowedSincPolyDataFilter smoother
#smoother SetInputConnection [surfreader GetOutputPort]
#smoother SetNumberOfIterations 50

#vtkPolyDataNormals surfnormals
#surfnormals SetInputConnection [smoother GetOutputPort]



# get the fluence data
vtkCellDataToPointData cell2point
cell2point SetInputData $td
cell2point PassCellDataOn

vtkContourFilter contours
contours SetInputConnection [cell2point GetOutputPort]
contours SetNumberOfContours 4
contours SetValue 0 1
contours SetValue 1 10
contours SetValue 2 100
contours SetValue 3 1000
contours Update

vtkWindowedSincPolyDataFilter contoursmoother
contoursmoother SetInputConnection [contours GetOutputPort]
contoursmoother SetNumberOfIterations 50

vtkPolyDataNormals contournormals
contournormals SetInputConnection [contoursmoother GetOutputPort]
contournormals Update

# create mapper for ordinary data
vtkDataSetMapper mapper
    mapper SetInputData $td
vtkActor actor
    actor SetMapper mapper
    [actor GetProperty] SetOpacity 0.2

# create contour mapper
vtkPolyDataMapper contourmapper
    contourmapper SetInputConnection [contournormals GetOutputPort]
vtkActor contouractor
    contouractor SetMapper contourmapper
    [contouractor GetProperty] SetOpacity 0.5


# render it
vtkRenderer ren
    ren AddActor actor
    ren AddActor contouractor

vtkRenderWindow renwin
    renwin AddRenderer ren

vtkRenderWindowInteractor iren
    iren SetRenderWindow renwin
    [iren GetInteractorStyle] SetCurrentStyleToTrackballCamera


renwin Render



