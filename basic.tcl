package require vtk

load libFullMonteVTK.so
load libFullMonteTIMOS_TCL.so
load libFullMonteBinFile_TCL.so 
load libFullMonteGeometry_TCL.so
load libFullMonteMatlabText_TCL.so

load libFullMonteKernel_TCL.so
load libFullMonteMatlabWriter_TCL.so

#default file prefix
set pfx "/usr/local/FullMonte/data/mouse"


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


set opt [R materials_simple]
set mesh [R mesh]

#VTKMeshRep V $mesh

BallSourceDescription bsr "10 40 11" 1.0 1.0
bsr setCentre "10 40 11"
bsr setRadius 2

# Load legend
set legend [R legend]
puts "Loaded [llength $legend] regions"

## Create sim kernel

TetraSurfaceKernel k


# Kernel properties
k setMesh               $mesh
k setSource bsr
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


proc progresstimer {} {
    global progress
    while { ![k done] } {
        after 50 {
            set progress [k getProgressFraction]
            puts "Progress update: $progress%"
            flush
        }
    }
}

proc showmaterial { mat } { 
    puts "Properties: mu_a=[SimpleMaterial_mu_a_get $mat]"
    puts "  mu_s=[SimpleMaterial_mu_s_get $mat] g=[SimpleMaterial_g_get $mat] n=[SimpleMaterial_n_get $mat]"
}

puts "opt(2) = [lindex $opt 2]"

showmaterial [lindex $opt 2]

set N 10
VTKSurfaceFluenceRep fluencerep V
vtkPolyDataWriter VTKW


MatlabWriter W
W setMesh $mesh
set surf [$mesh getRegionBoundaryTris 0]
W threshold 0
W setFaceSubset $surf
W writeFaces "faces.txt"

for { set i 0 } { $i < $N } { incr i } {

    puts "Trial $i of $N"
    
    # set the random seed
	k setRandSeed           $i

    # could equally well play with other stuff
    bsr setRadius [expr $i * 0.1 + 2.0]
    bsr setCentre "[expr $i * 1.0 + 10.0] 42 10"
    k setSource bsr

	k startAsync

    progresstimer

	k awaitFinish

    # get results
   fluencerep Update [k getSurfaceFluenceVector] 1

    # write the file out
    W setComment "Experiment $i"
    W writeSurfaceFluence "fluence.$i.txt" [k getSurfaceFluenceVector]

    VTKW SetHeader "Experiment run $i with blah blah blah"
	VTKW SetFileName "fluence.expt$i.vtk"
	VTKW SetInput [fluencerep getPolyData]
	VTKW Update
}
