package require vtk

proc loadRegion { fn actorName } {
    vtkXMLPolyDataReader reader$fn
    reader$fn SetFileName $fn

    vtkWindowedSincPolyDataFilter smoother$fn
    smoother$fn SetInputConnection [reader$fn GetOutputPort]
    smoother$fn SetNumberOfIterations 50
    
    vtkPolyDataNormals normals$fn
    normals$fn SetInputConnection [smoother$fn GetOutputPort]
    
    # create a mapper/actor to display the plain surface
    vtkPolyDataMapper surfacemapper$fn
    surfacemapper$fn SetInputConnection [normals$fn GetOutputPort]
    surfacemapper$fn ScalarVisibilityOff
    
    vtkActor $actorName
    $actorName SetMapper surfacemapper$fn
}


vtkRenderer ren
vtkRenderWindow renwin
renwin AddRenderer ren
vtkRenderWindowInteractor iren
iren SetRenderWindow renwin

loadRegion "output_stripped.xml" surf

ren AddActor surf
[surf GetProperty]  SetOpacity 0.1


for { set i 2 } { $i < 18 } { incr i } {
    loadRegion "output.surface$i.xml" surf$i
    [surf$i GetProperty] SetOpacity 0.5
}

renwin Render

set i 1

frame .showhide
pack .showhide

for { set i 2 } { $i < 18 } { incr i } {
    set show($i) 0
    button .showhide.org$i -text "Show organ $i" -command "
        if { \$show($i)==0 } {
            puts \"Showing organ $i\"
            .showhide.org$i configure -text \"Hide organ $i\"
            ren AddActor surf$i     
            set show($i) 1
        } else {
            puts \"Hiding organ $i\"
            .showhide.org$i configure -text \"Show organ $i\"
            ren RemoveActor surf$i
            set show($i) 0
        }
        renwin Render
    "
    pack .showhide.org$i
}


