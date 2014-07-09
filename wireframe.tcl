package require vtk

vtkXMLUnstructuredGridReader reader
reader SetFileName "volume_mesh.xml"

vtkXMLPolyDataReader surfreader
surfreader SetFileName "output_stripped.xml"

vtkWindowedSincPolyDataFilter smoother
smoother SetInputConnection [surfreader GetOutputPort]
smoother SetNumberOfIterations 50

vtkPolyDataNormals surfnormals
surfnormals SetInputConnection [smoother GetOutputPort]

proc setclipping { clipx clipy clipz {args ""} } {
    puts "Clipping plane set to $clipx $clipy $clipz and rendering"

    # create clipping plane
    plane SetOrigin $clipx $clipy $clipz
    plane SetNormal 0 0 1

    renwin Render
}

proc toggleclipper { } {
    global clip
    if {$clip == 0} {
        puts "Setting clipping ON"
        meshmapper SetInputConnection [clipper GetOutputPort]
        ren AddActor restactor
        set clip 1
    } else {
        set clip 0
        meshmapper SetInputConnection [reader GetOutputPort]
        ren RemoveActor restactor
        puts "Setting clipping OFF"
    }
    renwin Render
}

proc setscale { {args "" } } {
    global minf
    global maxf
    puts "Remapping scale to [expr pow(10,$minf)]-[expr pow(10,$maxf)]"
    lut SetTableRange [expr pow(10,$minf)] [expr pow(10,$maxf)]
    lut SetRange [expr pow(10,$minf)] [expr pow(10,$maxf)]
    meshmapper SetScalarRange [expr pow(10,$minf)] [expr pow(10,$maxf)]
    renwin Render
}

vtkPlane plane
    vtkClipDataSet clipper
        clipper SetInputConnection [reader GetOutputPort]
        clipper SetClipFunction plane
        clipper GenerateClippedOutputOff
        clipper SetValue 0.0

    vtkClipPolyData clipperInv
        clipperInv SetInputConnection [surfnormals GetOutputPort]
        clipperInv SetClipFunction plane
        clipperInv GenerateClippedOutputOff
        clipperInv SetValue 0.0
        clipperInv InsideOutOn

# create a mapper/actor to display the plain surface
vtkDataSetMapper meshmapper
meshmapper SetInputConnection [reader GetOutputPort]
meshmapper ScalarVisibilityOn

vtkDataSetMapper restmapper
restmapper SetInputConnection [clipperInv GetOutputPort]
restmapper ScalarVisibilityOff

vtkLookupTable lut
#    lut SetHueRange 0 0
#    lut SetValueRange 0 1
#    lut SetAlphaRange 1 1
#    lut SetSaturationRange 0 0
    lut SetScaleToLog10

meshmapper SetLookupTable lut

vtkActor meshactor
meshactor SetMapper meshmapper
#[meshactor GetProperty] SetRepresentationToWireframe
#[meshactor GetProperty] SetRepresentationToPoints
#[meshactor GetProperty] SetOpacity 0.1
#[meshactor GetProperty] BackfaceCullingOff
#[meshactor GetProperty] FrontfaceCullingOff

vtkActor restactor
restactor SetMapper restmapper
[restactor GetProperty] SetRepresentationToWireframe
[restactor GetProperty] SetOpacity 0.5

#vtkScalarBarActor bar
#bar SetLookupTable lut

# create renderer and go
vtkRenderer ren
ren AddActor meshactor
#ren AddActor restactor
#ren AddActor bar
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render

set clip 1

scale .clipx -label clipx -from -20 -to 20 -variable clipx -command {setclipping $clipx $clipy $clipz }
scale .clipy -label clipy -from -20 -to 20 -variable clipy -command {setclipping $clipx $clipy $clipz }
scale .clipz -label clipz -from -20 -to 20 -variable clipz -command {setclipping $clipx $clipy $clipz }

scale .minf -label "Min F" -from -10 -to 10 -variable minf -command { setscale }
scale .maxf -label "Max F" -from -10 -to 10 -variable maxf -command { setscale }

button .clip_en -text "Toggle clipping" -command {toggleclipper}

pack .clipx
pack .clipy
pack .clipz
pack .clip_en
pack .minf
pack .maxf

setclipping 0 0 0

iren Start


