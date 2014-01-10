# 
# First we include the VTK Tcl packages which will make available
# all of the VTK commands from Tcl. The vtkinteraction package defines
# a simple Tcl/Tk interactor widget.
# 
package require vtk 
package require vtkinteraction

# create sphere geometry 
vtkSphereSource sphere
sphere SetRadius 1.0
sphere SetThetaResolution 18 
sphere SetPhiResolution 18

# map to graphics library 
vtkPolyDataMapper map; 
map SetInput [sphere GetOutput]

# actor coordinates geometry, properties, transformation 
vtkActor aSphere 
aSphere SetMapper map 
[aSphere GetProperty] SetColor 0 0 1; # blue

# create a window to render into 
vtkRenderWindow renWin 
vtkRenderer ren1 
renWin AddRenderer ren1

# create an interactor 
vtkRenderWindowInteractor iren 
iren SetRenderWindow renWin

# add the sphere 
ren1 AddActor aSphere 
ren1 SetBackground 1 1 1;# Background color white

# Render an image; since no lights/cameras specified, created automatically 
renWin Render

# prevent the tk window from showing up then start the event loop 
wm withdraw .
