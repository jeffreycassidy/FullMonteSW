package require vtk

load libFullMonteVTK.so
load libFullMonteTIMOS_TCL.so
load libFullMonteBinFile_TCL.so

load libFullMonteKernel_TCL.so
load libFullMonteTIMOS_TCL.so

#default file prefix
set pfx "/Users/jcassidy/src/FullMonteSW/data/mouse"

#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "$pfx.opt"
set meshfn "$pfx.mesh"
set legendfn "$pfx.legend"



set ofn "fluence.out"

# create and set up the reader

TIMOSReader R

R setMeshFileName $meshfn
R setOpticalFileName $optfn
R setLegendFileName $legendfn



# Load materials

proc loadoptical { optfn } {
    global opt
    R setOpticalFileName $optfn

    set opt [R materials_simple]
}

loadoptical $optfn


# Load mesh

#set mesh [R mesh]

set meshfn "../Storage/BinFile/mouse"
#BinFileReader BR $meshfn
#set mesh [BR mesh]
set mesh [R mesh]
VTKMeshRep V $mesh

set ug [V getMeshWithRegions]
puts "Extracted cell mesh, total [$ug GetNumberOfCells] cells and [$ug GetNumberOfPoints] points"



# Load legend
set legend [R legend]
puts "Loaded [llength $legend] regions"

# actor: legend
for { set i 0 } { $i < [llength $legend] } { incr i } {
    V addLegendEntry [lindex $legend $i]
    puts "[LegendEntry_label_get [lindex $legend $i]]"
}

set legendactor [V getLegendActor "0.5 0.1" "0.9 0.9"]



## Create sim kernel

TetraVolumeKernel k

# Kernel properties
# k setSource bsr ## do this later
# k startAsync
k setEnergy             50
k setMaterials          $opt
k setUnitsToMM

# Monte Carlo kernel properties
k setRoulettePrWin      0.1
k setRouletteWMin       1e-3
k setMaxSteps           10000
k setMaxHits            100
k setThreadCount        8
k setRandSeed           1

# Tetra mesh MC kernel properties
k setMesh               $mesh


# set up VTK render window and interactor

vtkRenderer ren
    ren SetUseDepthPeeling 1
    ren SetMaximumNumberOfPeels 100
    ren SetOcclusionRatio 0.001

vtkRenderWindow renwin
    renwin AddRenderer ren

vtkRenderWindowInteractor iren
    iren SetRenderWindow renwin
    [iren GetInteractorStyle] SetCurrentStyleToTrackballCamera







# actor: mesh dataset

vtkDataSetMapper dsm
    dsm SetInputData $ug
    dsm SetLookupTable [V getRegionMapLUT]

vtkUnstructuredGridWriter W
    W SetInputData $ug
    W SetFileName "regions.vtk"
    W Update
W Delete

#vtkActor meshactor
#    meshactor SetMapper dsm
#    [meshactor GetProperty] SetColor 1.0 1.0 1.0

#ren AddActor meshactor




# actor: surface dataset

for { set i 0 } { $i < [llength $legend] } { incr i } {
    if { $i != 1 } {
        set surfpd($i) [V getSurfaceOfRegion $i]
        
        vtkPolyDataMapper pdm$i
            pdm$i SetInputData $surfpd($i)
            pdm$i ScalarVisibilityOff
        
        vtkActor surfactor$i
            surfactor$i SetMapper pdm$i
    
            eval "[surfactor$i GetProperty] SetColor [LegendEntry_colour_get [lindex $legend $i]]"


# treat exterior surface specially (lower opacity, white colour)
        if { $i == 0 } {
            [surfactor$i GetProperty] SetOpacity 0.2
            [surfactor$i GetProperty] SetColor 1.0 1.0 1.0
        } else { [surfactor$i GetProperty] SetOpacity 0.5 }

        ren AddActor surfactor$i
    }
}

VTKVolumeFluenceRep fluencerep V

ren AddViewProp $legendactor

set scalebar [fluencerep getScaleBar]
    $scalebar SetPosition 0.1  0.3
    $scalebar SetPosition2 0.2 0.7

ren AddViewProp $scalebar


set p0x 19
set p0y 14
set p0z 12 

set p1x 20
set p1y 22 
set p1z 13

VTKLineSourceRep lsr V "$p0x $p0y $p0z" "$p1x $p1y $p1z"

vtkLineWidget2 lsrwidget
    lsrwidget SetInteractor iren
    lsrwidget CreateDefaultRepresentation
    $ug ComputeBounds
    puts "Bounds are: [$ug GetBounds]"
    eval [lsrwidget GetRepresentation] PlaceWidget [$ug GetBounds]
    [lsrwidget GetRepresentation] SetPoint1WorldPosition $p0x $p0y $p0z
    [lsrwidget GetRepresentation] SetPoint2WorldPosition $p1x $p1y $p1z
    lsrwidget SetEnabled 1
    lsrwidget On

lsrwidget AddObserver InteractionEvent {
    puts "A: [[lsrwidget GetRepresentation] GetPoint1WorldPosition]"
    puts "B: [[lsrwidget GetRepresentation] GetPoint2WorldPosition]"
}


lsrwidget AddObserver EndInteractionEvent {
    set p0 [split [[lsrwidget GetLineRepresentation] GetPoint1WorldPosition]]
    set p1 [split [[lsrwidget GetLineRepresentation] GetPoint2WorldPosition]]
    puts "Start: $p0"
    puts "End:   $p1"

    set p0x [lindex $p0 1]
    set p0y [lindex $p0 2]
    set p0z [lindex $p0 3]

    set p1x [lindex $p1 1]
    set p1y [lindex $p1 2]
    set p1z [lindex $p1 3]


}

set lsactor [lsr getActor]
    [$lsactor GetProperty] SetOpacity 0.5

ren AddActor [lsrwidget GetRepresentation]





labelframe .input -text "Input problem definition"

label .input.mesh -text "Mesh filename: $meshfn"
pack .input.mesh

label .input.legend -text "Legend file: $legendfn"
pack .input.legend

frame .input.opt
label .input.opt.label -text "Optical properties: "
entry .input.opt.fn -textvariable optfn
pack .input.opt.label -side left
pack .input.opt.fn
pack .input.opt

pack .input


labelframe .source -text "Line source placement"

proc sourcecallback { } {
    global lsactor p0x p0y p0z p1x p1y p1z
    lsr endpoint 0 "$p0x $p0y $p0z"
    lsr endpoint 1 "$p1x $p1y $p1z"
    puts "Source moved to $p0x $p0y $p0z -- $p1x $p1y $p1z"
    lsr Update
    renwin Render
    return 1
}

frame .source.p0

entry .source.p0.x -validate focusout -vcmd sourcecallback -textvariable p0x
pack .source.p0.x

entry .source.p0.y -validate focusout -vcmd sourcecallback -textvariable p0y
pack .source.p0.y

entry .source.p0.z -validate focusout -vcmd sourcecallback -textvariable p0z
pack .source.p0.z

pack .source.p0

frame .source.p1
entry .source.p1.x -validate focusout -vcmd sourcecallback -textvariable p1x
pack .source.p1.x

entry .source.p1.y -validate focusout -vcmd sourcecallback -textvariable p1y
pack .source.p1.y

entry .source.p1.z -validate focusout -vcmd sourcecallback -textvariable p1z
pack .source.p1.z


pack .source.p1

pack .source

#scale .source.radius -label "Radius" -from 0.0 -to 10.0 -resolution 0.1 -orient horizontal -command sourcecallback
#pack .source.radius

#label .source.pos -text "Position: (+000.00 +000.00 +000.0)"
#pack .source.pos


pack .source

set Npkt 1000000

labelframe .options -text "Options"
label .options.packetlabel -text "Packets: "
pack .options.packetlabel
entry .options.packets -textvariable Npkt
pack .options.packets
pack .options


labelframe .output -text "Output file"

frame .output.fn


label .output.fn.fnlabel -text "Filename: "
pack .output.fn.fnlabel -side left

entry .output.fn.fntext -textvariable ofn
pack .output.fn.fntext

pack .output.fn

label .output.commentlabel -text "File comment text"
pack .output.commentlabel -side top

text  .output.comment 
pack .output.comment

ttk::progressbar .output.progress -maximum 100 -length 400 -variable progress
pack .output.progress

proc progresstimerevent {} {
    global phi_s progress
    after 50 {
        set progress [k getProgressFraction]
        puts "Progress update: $progress"
        if { ![k done] } { progresstimerevent } else {
            k awaitFinish
            onSimFinish
        }
    }
}

proc onSimFinish {} {
    set phi_s [k getVolumeFluenceVector]
    fluencerep Update "$phi_s"
    [fluencerep getData] Modified

    global clipper

    if { [llength [info commands fluencecutactor]] == 0 } {
        # clip the fluence along a plane
#        vtkClipDataSet fluenceclipper
#            fluenceclipper SetInputData []
#            fluenceclipper SetClipFunction

        vtkPlane fluencecutplane
            fluencecutplane SetNormal 1 0 0
            fluencecutplane SetOrigin 19 14 12

        vtkCutter fluencecutter
            fluencecutter SetInputData [fluencerep getData]
            fluencecutter SetValue 0 0
            fluencecutter SetGenerateTriangles 1
            fluencecutter SetCutFunction fluencecutplane
            fluencecutter GenerateCutScalarsOff

        vtkPolyDataMapper fluencecutmapper
            fluencecutmapper SetInputConnection [fluencecutter GetOutputPort]
            fluencecutmapper ScalarVisibilityOn
#            puts "lut: [fluencerep getLookupTable]"
#            puts "data range: [fluencerep getRange]"
#            [fluencerep getLookupTable] AddRGBSegment 0.0 0 0 0   500  0   0 1.0
#            [fluencerep getLookupTable] AddRGBSegment 500 0 0 1.0 1000 1.0  0 0.0

            set r [fluencerep getRange]

            fluencecutmapper SetLookupTable [fluencerep getLookupTable]
            fluencecutmapper SetScalarRange 0 [lindex $r 1]
            fluencecutmapper SetUseLookupTableScalarRange 0

        vtkActor fluencecutactor
            fluencecutactor SetMapper fluencecutmapper

        ren AddActor fluencecutactor
        renwin Render
    } else {
        renwin Render
    }

#    ren AddActor [fluencerep getActor]
#    renwin Render

    set ofn "lastrender.vtk"

    vtkUnstructuredGridWriter W
    W SetFileName $ofn
    W SetInputData [fluencerep getData]
    W Update
    W Delete
    puts "Wrote to $ofn"
}

button .output.save -text "Calculate & Save" -command {
    global mesh opt ofn progress
    set progress 0
    k setSource [lsr getDescription]
    k setPacketCount        $Npkt
    k startAsync

    progresstimerevent
}
pack .output.save

pack .output

# add fluence toggle button
set showfluence 0
proc fluencecallback {} {
    global showfluence
    if { $showfluence==0 } {
        set showfluence 1
        .fluenceenable configure -text "Hide fluence"
        ren AddActor fluencecutactor
        renwin Render
    } else {
        set showfluence 0
        .fluenceenable configure -text "Show fluence"
        ren RemoveActor fluencecutactor
        renwin Render
    }
}
button .fluenceenable -text "Show Fluence" -command { fluencecallback }
pack .fluenceenable

ren ResetCamera
renwin Render
puts "Depth peeling: [ren GetLastRenderingUsedDepthPeeling]"
