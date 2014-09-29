package require vtk

# load data
load TetraMeshTCL.so

set fn "data/mouse.mesh"

# load from text mesh file
set imesh [loadMeshFile $fn]
puts "data loaded from $fn";

set mesh [buildMesh $imesh]

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
    boxrep_$name PlaceWidget 8 30 33 48 4 19
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

vtkLegendBoxActor legendactor
    legendactor SetNumberOfEntries 19
    [legendactor GetPositionCoordinate] SetCoordinateSystemToView
    [legendactor GetPositionCoordinate] SetValue 0.5 0.0
    [legendactor GetPosition2Coordinate] SetCoordinateSystemToView
    [legendactor GetPosition2Coordinate] SetValue 0.9 0.9

    legendactor SetEntryString  0 "(outside DVH box)"
    legendactor SetEntryString  1 "Muscle"
    legendactor SetEntryString  2 "Brain"
    legendactor SetEntryString  3 "Heart"
    legendactor SetEntryString  4 "Bladder"
    legendactor SetEntryString  5 "Stomach"
    legendactor SetEntryString  6 "Spleen"
    legendactor SetEntryString  7 "Kidney"
    legendactor SetEntryString  8 "Kidney"
    legendactor SetEntryString  9 "Lung"
    legendactor SetEntryString 10 "Lung"
    legendactor SetEntryString 11 "Pancreas"
    legendactor SetEntryString 12 "Liver"
    legendactor SetEntryString 13 "Skeleton"
    legendactor SetEntryString 14 "Muscle"
    legendactor SetEntryString 15 "Muscle"
    legendactor SetEntryString 16 "Testis"
    legendactor SetEntryString 17 "Testis"
    legendactor SetEntryString 18 "Tumour"

    legendactor SetEntryColor  1  1.0 1.0 1.0
    legendactor SetEntryColor  2  0.0 1.0 0.0
    legendactor SetEntryColor  3  0.0 0.0 1.0
    legendactor SetEntryColor  4  1.0 1.0 0.0
    legendactor SetEntryColor  5  1.0 0.0 1.0
    legendactor SetEntryColor  6  0.0 1.0 1.0
    legendactor SetEntryColor  7  1.0 1.0 0.0
    legendactor SetEntryColor  8  0.5 1.0 1.0
    legendactor SetEntryColor  9  1.0 0.5 1.0
    legendactor SetEntryColor 10  1.0 1.0 0.5
    legendactor SetEntryColor 11  0.5 0.5 1.0
    legendactor SetEntryColor 12  0.5 1.0 0.5
    legendactor SetEntryColor 13  1.0 0.5 0.5
    legendactor SetEntryColor 14  0.0 0.0 0.5
    legendactor SetEntryColor 15  0.0 0.5 0.0
    legendactor SetEntryColor 16  0.5 0.0 0.0
    legendactor SetEntryColor 17  0.5 0.5 0.0

    legendactor SetEntryColor 18  1.0 0.0 0.0



#    lut SetAnnotation 0 "Exterior"
#    lut SetAnnotation 1 "Muscle"
#    lut SetAnnotation 2 "Brain"
#    lut SetAnnotation 3 "Heart"
#    lut SetAnnotation 4 "Bladder"
#    lut SetAnnotation 5 "Stomach"
#    lut SetAnnotation 6 "Spleen"
#    lut SetAnnotation 7 "Kidney"
#    lut SetAnnotation 8 "Kidney"
#    lut SetAnnotation 9 "Lung"
#    lut SetAnnotation 10 "Lung"
#    lut SetAnnotation 11 "Pancreas"
#    lut SetAnnotation 12 "Liver"
#    lut SetAnnotation 13 "Skeleton"
#    lut SetAnnotation 14 "Muscle"
#    lut SetAnnotation 15 "Muscle"
#    lut SetAnnotation 16 "Testis"
#    lut SetAnnotation 17 "Testis"
#    lut SetAnnotation 18 "Tumour"




# set up rendering window
vtkRenderer ren
vtkRenderWindow renwin
renwin AddRenderer ren

ren AddActor legendactor

for { set i 0 } { $i < 18 } { incr i } {
    addSurf surf$i $mesh $i ren
}

#[surf0 GetProperty] SetColor 1.0 1.0 1.0
#[surf0 GetProperty] SetOpacity 0.2

ren RemoveActor surf0
ren AddActor tumour
ren AddActor clipactor

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

#iren Start
