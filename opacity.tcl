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


# create a mapper/actor with LUT to display the surface emittance
vtkCamera camera
vtkDepthSortPolyData sorter
    sorter SetInputConnection [normals GetOutputPort]
    sorter SetDirectionToBackToFront
    sorter SetVector 1 0 0
    sorter SortScalarsOn
    sorter SetCamera camera

vtkPolyDataMapper emittancemapper
emittancemapper SetInputConnection [normals GetOutputPort]
    emittancemapper SetScalarRange 0 1e-3

vtkLookupTable lut
    lut SetScaleToLog10
    lut SetValueRange 0.5 1.0
    lut Build
    lut SetTableValue 0 1 1 1 0.1

emittancemapper  SetLookupTable lut

vtkActor emittanceactor
emittanceactor SetMapper emittancemapper

# create renderer and go
vtkRenderer ren
#ren SetUseDepthPeeling 1
#ren SetMaximumNumberOfPeels 200
#ren SetOcclusionRatio 0.001
#ren AddActor surfaceactor
#ren SetActiveCamera camera
ren AddActor emittanceactor
vtkRenderWindow renwin
renwin AddRenderer ren

puts "Depth peeling: [ren GetLastRenderingUsedDepthPeeling]"

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render

wm withdraw .

iren Start


