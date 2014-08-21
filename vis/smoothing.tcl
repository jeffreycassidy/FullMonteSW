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

# create a mapper/actor with LUT to display the surface emittance
vtkPolyDataMapper emittancemapper
emittancemapper SetInputConnection [normals GetOutputPort]
emittancemapper SetScalarRange 0 1e-3

vtkLookupTable lut
lut SetScaleToLog10
lut SetValueRange 0.5 1.0
emittancemapper  SetLookupTable lut

vtkActor emittanceactor
emittanceactor SetMapper emittancemapper

vtkRenderer ren
ren AddActor surfaceactor
ren AddActor emittanceactor
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render
iren Start

wm withdraw .

