package require vtk

# load data
load TetraMeshTCL.so
set conn [tclConnect]
set mesh [loadMesh $conn 1]
set ts   [extractBoundary $mesh 3]

proc addSurf { name mesh matID ren } {
    createVTKBoundary pd_$name $mesh $matID
    vtkPolyDataMapper map_$name
    map_$name SetInputData pd_$name
    vtkActor $name
    [$name GetProperty] SetOpacity 0.5
    $name SetMapper map_$name
    $ren AddActor $name
}

proc lineWidgetUpdate { name } {
    global $name
    global linerep_$name
    set p0 [linerep_$name GetPoint1WorldPosition]
    set p1 [linerep_$name GetPoint2WorldPosition]
    puts "Line $name updated to new position $p0-$p1"
}

proc addLineProbe { name iren } {
    vtkLineWidget2 widget_$name
    vtkLineRepresentation linerep_$name
    widget_$name SetRepresentation linerep_$name
    widget_$name AddObserver InteractionEvent "lineWidgetUpdate $name"
    widget_$name SetInteractor $iren
    linerep_$name SetPoint1WorldPosition 18.5 23.6 13.9
    linerep_$name SetPoint2WorldPosition 20 15 13
    widget_$name SetEnabled 1
    }

# set up rendering window
vtkRenderer ren
vtkRenderWindow renwin
renwin AddRenderer ren

for { set i 0 } { $i < 18 } { incr i } {
    addSurf surf$i $mesh $i ren
}

#[surf0 GetProperty] SetColor 1.0 1.0 1.0
#[surf0 GetProperty] SetOpacity 0.2

ren RemoveActor surf0

[surf1 GetProperty] SetColor 1.0 1.0 1.0
[surf1 GetProperty] SetOpacity 0.2

[surf2 GetProperty] SetColor 0.0 1.0 0.0
[surf3 GetProperty] SetColor 0.0 0.0 1.0
[surf4 GetProperty] SetColor 1.0 1.0 0.0
[surf5 GetProperty] SetColor 1.0 0.0 1.0
[surf6 GetProperty] SetColor 0.0 1.0 1.0
[surf7 GetProperty] SetColor 1.0 1.0 0.0
[surf8 GetProperty] SetColor 0.5 1.0 1.0
[surf9 GetProperty] SetColor 1.0 0.5 1.0
[surf10 GetProperty] SetColor 1.0 1.0 0.5
[surf11 GetProperty] SetColor 0.5 0.5 1.0
[surf12 GetProperty] SetColor 0.5 1.0 0.5
[surf13 GetProperty] SetColor 1.0 0.5 0.5
[surf14 GetProperty] SetColor 0.0 0.0 0.5
[surf15 GetProperty] SetColor 0.0 0.5 0.0
[surf16 GetProperty] SetColor 0.5 0.0 0.0
[surf17 GetProperty] SetColor 0.5 0.5 0.0

# start render interactor
vtkRenderWindowInteractor iren

# add interaction widgets
addLineProbe lp0 iren
ren AddActor linerep_lp0


[iren GetInteractorStyle] SetCurrentStyleToTrackballCamera
iren SetRenderWindow renwin
renwin Render
