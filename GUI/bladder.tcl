package require vtk

load libFullMonteVTK.so
load libFullMonteTIMOS_TCL.so
load libFullMonteVolume_TCL.so
load libFullMonteBinFile_TCL.so

# load common GUI elements
source commongui.tcl

# create basic VTK window with ren, renwin, iren
doVTKWindow



#default file prefix
set pfx "/Users/jcassidy/src/FullMonteSW/humandata/bladder"

#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set meshfn "$pfx.mesh"
set legendfn "$pfx.legend"
set optfn "$pfx.opt"

set ofn "fluence.out"


# create and set up the reader

TIMOSReader R

R setMeshFileName $meshfn
R setOpticalFileName $optfn
R setLegendFileName $legendfn



# Load materials
set mats [R materials_simple]
puts "Read [llength $mats] materials"


# Load legend
set legend [R legend]
puts "Loaded [llength $legend] regions"


# Load mesh and create VTK representation
#set mesh [R mesh]
BinFileReader BR "../Storage/BinFile/bladder"
set mesh [BR mesh]
VTKMeshRep V $mesh

set ug [V getMeshWithRegions]
puts "Extracted cell mesh, total [$ug GetNumberOfCells] cells and [$ug GetNumberOfPoints] points"


# Create GUI

global phimin phimax
set phimin 0.0
set phimax 50.0

proc updatescale {} {
    if { [info exists ::phi_v]==1 } {
        global phi_v phimin phimax
        fluencerep setRange $phimin $phimax
        fluencerep Update $phi_v
        renwin Render
    } else {
        puts "INFO: updatescale{}: No fluence vector yet"
    }
    return 1
}

global scalevar
set scalevar "cm"

labelframe .geom -text "Problem geometry"
makeMeshFrame .geom $meshfn $mesh V scalevar
pack .geom

labelframe .source -text "Source properties"
makePointSourceFrame .source pos
pack .source

labelframe .opt -text "Optical properties"
makeOpticalFrame .opt $mats $legend
pack .opt

labelframe .sim -text "Simulation options"
makeSimFrame .sim doSim
pack .sim

global phi_v

proc doSim {} {
    global mesh mats src scalevar phi_v E
    for { set i 0 } { $i < [llength $mats] } { incr i } {
        foreach p [list mu_a mu_s g n] {
            set $p [.opt.mats.$p$i get]
            SimpleMaterial_${p}_set [lindex $mats $i] [.opt.mats.$p$i get]
        }
    }
    
    if { $scalevar == "mm" } { set units_per_cm 10 } elseif { $scalevar == "cm" } { set units_per_cm 1 } else {
        puts "WARNING: Invalid scale unit $scalevar" }

    set phi_v [VolumeKernel $mesh $mats [psr getSourceDescription] [.sim.opts.npkt get] $units_per_cm $E]
    fluencerep Update $phi_v
    
    renwin Render
}



# actor: legend
for { set i 0 } { $i < [llength $legend] } { incr i } {
    V addLegendEntry [lindex $legend $i]
    puts "[LegendEntry_label_get [lindex $legend $i]]"
}

set legendactor [V getLegendActor "0.5 0.1" "0.9 0.9"]






# actor: mesh dataset

vtkDataSetMapper dsm
    dsm SetInputData $ug
    dsm SetLookupTable [V getRegionMapLUT]

vtkActor meshactor
    meshactor SetMapper dsm
    [meshactor GetProperty] SetColor 1.0 1.0 1.0

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

        vtkPolyDataWriter pdw
        pdw SetInputData $surfpd($i)
        pdw SetFileName "surface.$i.vtk"
        pdw Write
        pdw Delete


# treat exterior surface specially (lower opacity, white colour)
        if { $i == 0 } {
            [surfactor$i GetProperty] SetOpacity 0.2
            [surfactor$i GetProperty] SetColor 1.0 1.0 1.0
        } else { [surfactor$i GetProperty] SetOpacity 0.5 }

        ren AddActor surfactor$i
    }
}

# actor: surface fluence dataset

#VTKSurfaceFluenceRep fluencerep V

VTKVolumeSurfaceRep fluencerep V 2 1

set fluenceactor [fluencerep getActor]
ren AddActor $fluenceactor

#set fluenceactor [fluencerep getActor]
#    ren AddActor $fluenceactor


ren AddViewProp $legendactor

set scalebar [fluencerep getScaleBar]
    $scalebar SetPosition 0.1  0.3
    $scalebar SetPosition2 0.15 0.7

ren AddViewProp $scalebar



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




####################################################################################################################################
## Point source rep with callback on update

VTKPointSourceRep psr V "1.0 2.0 3.0"

[psr getWidget] SetInteractor iren
[psr getWidget] SetEnabled 1
psr Update

[psr getWidget] AddObserver InteractionEvent {
    set pos [psr getPosition]

    set px [format "%6.3f" [lindex $pos 0]]
    set py [format "%6.3f" [lindex $pos 1]]
    set pz [format "%6.3f" [lindex $pos 2]]

    puts "Source position updated to $px $py $pz"

    foreach c [list x y z] { .source.pos.$c delete 0 end; eval ".source.pos.$c insert 0 \$p$c" }
}






####################################################################################################################################
## Final render before entering event loop

renwin Render
puts "Depth peeling: [ren GetLastRenderingUsedDepthPeeling]"
