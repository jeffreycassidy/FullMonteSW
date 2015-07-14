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


set opt [R materials_simple]
set mesh [R mesh]

#set meshfn "/home/houmanhaji/FullMonte/data/DATA/mouse.bin"
#BinFileReader BR $meshfn
#set mesh [BR mesh]

BallSourceDescription bsr
bsr setCentre 1 1 1
bsr setRadius 1

# Load legend
set legend [R legend]
puts "Loaded [llength $legend] regions"

## Create sim kernel

TetraSurfaceKernel k


# Kernel properties
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


proc progresstimerevent {} {
    global phi_s progress
    after 50 {
        set progress [k getProgressFraction]
        puts "Progress update: $progress"
        if { ![k done] } { progresstimerevent } else {
            k awaitFinish
            set phi_s [k getSurfaceFluenceVector]
        }
    }
}

for { set i 0 } { $i < 10 } { incr i } {
	k setRandSeed           $i
	k setOutputFile "experiment.$i.out"
	k startAsync
	k awaitFinish
}


# Tetra mesh MC kernel properties
k setMesh               $mesh


    k setSource [bsr getDescription]
    k setPacketCount 1000000
    k startAsync
    progresstimerevent

VTKBallSourceRep bsr V "1.0 1.0 1.0" 10.0

