package require vtk

# load data
load TetraMeshTCL.so

set fn "data/mouse.mesh"

# load from text mesh file
set mesh [loadMesh $fn]
puts "data loaded from $fn";

# load from database
#set conn [tclConnect]
#set mesh [loadMesh $conn 1]
#set ts   [extractBoundary $mesh 3]

## Load the tumour region
set tumourtets [loadVector "tumour_tet_IDs.txt"]

set pd_tumour [getVTKRegion $mesh $tumourtets]

vtkPolyDataMapper map_tumour
map_tumour SetInputData $pd_tumour
vtkActor tumour
[tumour GetProperty] SetOpacity 0.8
[tumour GetProperty] SetColor 1.0 0.0 0.0
tumour SetMapper map_tumour


proc addSurf { name mesh matID ren } {
    puts "Adding surface $matID"
    set pd [createVTKBoundary $mesh $matID]
    set pd_$name $pd
    vtkPolyDataMapper map_$name
    map_$name SetInputData $pd
    vtkActor $name
    [$name GetProperty] SetOpacity 0.3
    $name SetMapper map_$name
    $ren AddActor $name
    puts "Done"
}

proc boxWidgetUpdate { name } {
    boxrep_$name GetPlanes planes_$name

    puts "PTV bounding box: "
    for { set i 0 } { $i < 6 } { incr i } {
        set pl [planes_$name GetPlane $i]
        puts "  Plane $i: n=[$pl GetNormal] p=[$pl GetOrigin]"
    }
}

proc addBoxBound { name ren iren } {
    vtkPlanes planes_$name

    vtkBoxWidget2 widget_$name
    widget_$name SetInteractor $iren
    widget_$name EnabledOn
    widget_$name TranslationEnabledOn
    widget_$name ScalingEnabledOn
    widget_$name RotationEnabledOn

    widget_$name AddObserver EndInteractionEvent "boxWidgetUpdate $name"

    vtkBoxRepresentation boxrep_$name
    widget_$name SetRepresentation boxrep_$name
    boxrep_$name SetPlaceFactor 1.0
    boxrep_$name PlaceWidget 8 30 33 48 4 19
    boxrep_$name InsideOutOn
    # get normals pointing in

    $ren AddActor boxrep_$name
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
    linerep_$name SetPoint1WorldPosition 11.4 42.4 10.4
    linerep_$name SetPoint2WorldPosition 11.7 41.6 11.0
    widget_$name SetEnabled 1
    }


button .run -text "Run" -command {
    set p1 [linerep_lp0 GetPoint1WorldPosition]
    set p2 [linerep_lp0 GetPoint1WorldPosition]
    puts "montecarlo --input data/mouse.mesh --N 100000 --source 'line:1.0:$p1 $p2'" }

pack .run


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
ren AddActor tumour

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

[iren GetInteractorStyle] SetCurrentStyleToTrackballCamera
iren SetRenderWindow renwin


# add interaction widgets
renwin Render

addLineProbe lp0 iren
addBoxBound bx0 ren iren
ren AddActor linerep_lp0

renwin Render

puts "rendered"

iren Start
