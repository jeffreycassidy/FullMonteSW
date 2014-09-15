package require vtk
load TetraMeshTCL.so

set conn [tclConnect]
set mesh [loadMesh $conn 1]

#set ts [extractBoundary $mesh 1]

#set pd [getVTKPolyData $ts]

#puts $pd

#### Solid rendering

set voldata [getVTKTetraData $mesh]

vtkPiecewiseFunction opacityfn
#    opacityfn AddSegment 0 1.0 255 1.0
    opacityfn AddSegment 0 0.0 18 1.0

vtkDiscretizableColorTransferFunction colorlut
    colorlut SetRange 0 18
#    colorlut SetColorSpaceToRGB
#    colorlut DiscretizeOn
#    colorlut SetNumberOfIndexedColors 18
#    colorlut EnableOpacityMappingOn
#    colorlut IndexedLookupOn
##    colorlut Build
#
#    colorlut SetIndexedColor 0 1.0 1.0 1.0
#    colorlut SetIndexedColor 1 1.0 1.0 1.0
#    colorlut SetIndexedColor 2 1.0 0.0 0.0
#    colorlut SetIndexedColor 3 0.0 1.0 0.0
#    colorlut SetIndexedColor 4 0.0 0.0 1.0
#    colorlut SetIndexedColor 5 1.0 1.0 0.0
#    colorlut SetIndexedColor 6 0.0 1.0 1.0
#    colorlut SetIndexedColor 7 1.0 0.0 1.0
#
#    colorlut SetIndexedColor 8 1.0 0.0 0.0
#    colorlut SetIndexedColor 9 1.0 0.0 0.0
#    colorlut SetIndexedColor 10 1.0 0.0 0.0
#    colorlut SetIndexedColor 11 1.0 0.0 0.0
#    colorlut SetIndexedColor 12 1.0 0.0 0.0
#    colorlut SetIndexedColor 13 1.0 0.0 0.0
#    colorlut SetIndexedColor 14 1.0 0.0 0.0
#    colorlut SetIndexedColor 15 1.0 0.0 0.0
#    colorlut SetIndexedColor 16 1.0 0.0 0.0
#    colorlut SetIndexedColor 17 1.0 0.0 0.0
#
#    for { set i 0 } { $i < 18 } {incr i } { colorlut SetAnnotation $i "foo" }


vtkVolumeProperty volprop
#    volprop SetColor colorlut
    volprop SetScalarOpacity opacityfn
    volprop ShadeOn
    volprop SetInterpolationTypeToLinear
    volprop SetDiffuse 0.7
    volprop SetAmbient 0.1
    volprop SetSpecular 0.5
    volprop SetSpecularPower 70.0

#vtkVolumeRayCastCompositeFunction compfn
vtkUnstructuredGridVolumeRayCastMapper volmapper
    volmapper SetInputData $voldata

vtkVolume volume
    volume SetProperty volprop
    volume SetMapper volmapper

# set up rendering window
vtkRenderer ren
vtkRenderWindow renwin

vtkLight light
    light SetColor 1 1 1
    light SetPosition 10 20 30
    light SetFocalPoint 10 20 10
    light SetIntensity 1.0

ren AddLight light
ren AddActor volume

vtkDataSetMapper dsm
    dsm SetInputData $voldata
    dsm SetLookupTable colorlut
vtkActor act2
    act2 SetMapper dsm
    [act2 GetProperty] SetOpacity 1.0
    [act2 GetProperty] SetAmbient 0.0
    [act2 GetProperty] SetDiffuse 0.0

#ren AddActor act2

renwin AddRenderer ren


# put actors in scene
#set name foo

#vtkPolyDataMapper map_$name
#    map_$name SetInputData $pd
#    vtkActor $name
#    [$name GetProperty] SetOpacity 0.5
#    $name SetMapper map_$name
#    ren AddActor $name



# start render interactor
vtkRenderWindowInteractor iren

[iren GetInteractorStyle] SetCurrentStyleToTrackballCamera
iren SetRenderWindow renwin
renwin Render
iren Start
