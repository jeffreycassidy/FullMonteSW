package require vtk
load TetraMeshTCL.so

set M [loadTetraMeshBaseText "data/mouse.mesh"]
set tets [getVTKTetraMesh $M]

#debug output
#saveTetraMeshBaseText $M {tetras.out}

vtkDataSetMapper mapper
    mapper SetInputData $tets

puts "Dataset loaded with [$tets GetNumberOfCells] cells and [$tets GetNumberOfPoints] points"

vtkExtractEdges edges
    edges SetInputData $tets

vtkPolyDataMapper edgemapper
    edgemapper SetInputConnection [edges GetOutputPort]

vtkActor edgeactor
    edgeactor SetMapper edgemapper

vtkActor actor
    actor SetMapper mapper
    [actor GetProperty] SetRepresentationToWireframe

# start render interactor
vtkRenderer ren
vtkRenderWindow renwin
renwin AddRenderer ren

# add actors to scene
#ren AddActor actor
ren AddActor edgeactor


vtkRenderWindowInteractor iren
[iren GetInteractorStyle] SetCurrentStyleToTrackballCamera
iren SetRenderWindow renwin
renwin Render
#iren start
