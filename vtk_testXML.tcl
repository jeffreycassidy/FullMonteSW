vtkXMLPolyDataReader reader
reader SetFileName "output.xml"
vtkPolyDataMapper readmapper
readmapper SetInputConnection [reader GetOutputPort]
vtkActor readactor
readactor SetMapper readmapper
vtkRenderer ren
ren AddActor readactor
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render
iren Start
