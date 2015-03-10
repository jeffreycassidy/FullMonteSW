package require vtk

# load data
load TetraMeshTCL.so

TIMOSReader R "data/mouse"

set M [R mesh]
set mat [R materials]
set src [R sources]
set legend [R legend]



# load from database
#set conn [tclConnect]
#set mesh [loadMesh $conn 1]
#set ts   [extractBoundary $mesh 3]

## Load the tumour region
#set tumourtets [loadVector "tumour_tet_IDs.txt"]

#set pd_tumour [getVTKRegion $mesh $tumourtets]

vtkPolyDataMapper map_tumour
map_tumour SetInputData $pd_tumour
vtkActor tumour
[tumour GetProperty] SetOpacity 0.8
[tumour GetProperty] SetColor 1.0 0.0 0.0
tumour SetMapper map_tumour


proc addSurf { name mesh matID ren } {
    set pd [createVTKBoundary $mesh $matID]
    set pd_$name $pd

    vtkWindowedSincPolyDataFilter smoother_$name
    smoother_$name SetInputData $pd
    smoother_$name SetNumberOfIterations 50

    vtkPolyDataNormals normals_$name
    normals_$name SetInputConnection [smoother_$name GetOutputPort]

    vtkPolyDataMapper map_$name
    map_$name SetInputConnection [normals_$name GetOutputPort]

    vtkActor $name
    [$name GetProperty] SetOpacity 0.3
    $name SetMapper map_$name
    $ren AddActor $name
}

proc boxWidgetUpdate { name } {
    puts "PTV bounding box: [getPlaneExpression $name]"
}

proc getPlaneExpression { name } {
    vtkPlanes tmppl
    boxrep_$name GetPlanes tmppl

    for { set i 0 } { $i < 6 } { incr i } {
        set pl [tmppl GetPlane $i]
        set n$i [$pl GetNormal]
        set c$i [$pl EvaluateFunction 0 0 0]
    }

    tmppl Delete

    return "$n0 [expr -$c0] $c1 $n2 [expr -$c2] $c3 $n4 [expr -$c4] $c5"
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
    boxrep_$name PlaceWidget 7 17 35.5 45.5 6.5 16.5
    boxrep_$name InsideOutOn
    # get normals pointing in

    $ren AddActor boxrep_$name
}

proc updateDVHBox { } {
    global imesh
    set pp [readParallelepiped [getPlaneExpression bx0]]
    set cliptets [clipToRegion $imesh $pp]
    clipmapper SetInputData [getVTKTetras $cliptets]
    clipmapper Update
    ren Render
}

vtkDataSetMapper clipmapper

vtkActor clipactor
    clipactor SetMapper clipmapper


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

# put in the legend
set legendactor [legend getActor]
ren AddActor legendactor

# add surface actors
for { set i 0 } { $i < 18 } { incr i } {
    addSurf surf$i $mesh $i ren
    [surf$i GetProperty] SetColor [legend getColour $i]
}

[surf1 GetProperty] SetOpacity 0.2

ren RemoveActor surf0
ren AddActor tumour
ren AddActor clipactor

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

#iren Start
