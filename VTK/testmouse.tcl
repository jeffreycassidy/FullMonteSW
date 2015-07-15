package require vtk

load libFullMonteVTK.so
load libFullMonteTIMOS_TCL.so
load libFullMonteBinFile_TCL.so 

load libFullMonteKernels_TCL.so
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

set mesh [R mesh]

set meshfn "/home/houmanhaji/FullMonte/data/DATA/mouse.bin"
#BinFileReader BR $meshfn
#set mesh [BR mesh]
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

TetraSurfaceKernel k


# Kernel properties
# k setSource bsr ## do this later
# k startAsync
k setEnergy             50
k setMaterials          $opt
k setUnitsToMM


# Monte Carlo kernel properties
k setRoulettePrWin      0.1
k setRouletteWMin       1e-5
k setMaxSteps           10000
k setMaxHits            100
k setPacketCount        1000000
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

vtkActor meshactor
    meshactor SetMapper dsm
    [meshactor GetProperty] SetColor 1.0 1.0 1.0

ren AddActor meshactor



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

# actor: surface fluence dataset

VTKSurfaceFluenceRep fluencerep V

set fluenceactor [fluencerep getActor]
    ren AddActor $fluenceactor


ren AddViewProp $legendactor

set scalebar [fluencerep getScaleBar]
    $scalebar SetPosition 0.1  0.3
    $scalebar SetPosition2 0.2 0.7

ren AddViewProp $scalebar


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


labelframe .source -text "Source placement"

proc scalecallback { newval } { global bsractor; bsr setRadius $newval; bsr Update; renwin Render; }

scale .source.radius -label "Radius" -from 0.0 -to 10.0 -resolution 0.1 -orient horizontal -command scalecallback

pack .source.radius

label .source.pos -text "Position: (+000.00 +000.00 +000.0)"

pack .source.pos


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
            set phi_s [k getSurfaceFluenceVector]
            fluencerep Update $phi_s 1
            renwin Render
        }
    }
}

button .output.save -text "Calculate & Save" -command {
    global mesh opt ofn progress
    set progress 0
    k setSource [bsr getDescription]
    k setPacketCount $Npkt
    k startAsync

    progresstimerevent
}
pack .output.save

pack .output

# add fluence toggle button
set showfluence 0
proc fluencecallback {} {
    global showfluence fluenceactor
    if { $showfluence==0 } {
        set showfluence 1
        .fluenceenable configure -text "Hide fluence"
        ren AddActor $fluenceactor
        renwin Render
    } else {
        set showfluence 0
        .fluenceenable configure -text "Show fluence"
        ren RemoveActor $fluenceactor
        renwin Render
    }
}
button .fluenceenable -text "Show Fluence" -command { fluencecallback }
pack .fluenceenable


# add placement interactor
vtkPointWidget pointwidget
    pointwidget OutlineOn
    pointwidget ZShadowsOn
    pointwidget XShadowsOn
    pointwidget YShadowsOn
    pointwidget SetInteractor iren
    pointwidget SetEnabled 1
    $ug ComputeBounds
    puts "Bounds: [$ug GetBounds]"
    eval pointwidget PlaceWidget [$ug GetBounds]

pointwidget AddObserver EndInteractionEvent {
    set pos [eval "format {Position: (%6.2f %6.2f %6.2f)} [pointwidget GetPosition]"]
    .source.pos configure -text $pos
    bsr setCentre [pointwidget GetPosition];
    bsr Update;
    renwin Render;
}


VTKBallSourceRep bsr V "1.0 1.0 1.0" 10.0

set bsractor [bsr getActor]
    [$bsractor GetProperty] SetOpacity 0.5

ren AddActor $bsractor


renwin Render
puts "Depth peeling: [ren GetLastRenderingUsedDepthPeeling]"
