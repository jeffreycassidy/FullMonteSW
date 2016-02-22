load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteVTKFileTCL.so
load libFullMonteKernelsTCL.so

#default file prefix
set dir "/Users/jcassidy/src/TLT_Knee/"

#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "${dir}MuscleFatSplit660Light.opt"
set meshfn "(none)"
set legendfn "${dir}MuscleFatSplit.legend"

# create and set up the reader

VTKLegacyReader VR

VR setFileName "${dir}MuscleFatSplit.mesh.vtk"

set mesh [VR mesh]

puts "Loaded mesh"

TIMOSReader TR
TR setOpticalFileName $optfn
TR setLegendFileName $legendfn

set opt [TR materials_simple]


BallSourceDescription bsr "10 40 11" 1.0 1.0
bsr setCentre "10 40 11"
bsr setRadius 2

# Load legend
set legend [TR legend]
puts "Loaded [llength $legend] regions"

## Create sim kernel

TetraSurfaceKernel k


# Kernel properties
k setMesh               $mesh
k setSource bsr
k setEnergy             50
k setMaterials          $opt
k setUnitsToCM


# Monte Carlo kernel properties
k setRoulettePrWin      0.1
k setRouletteWMin       1e-3
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

#VTKSurfaceFluenceRep fluencerep V
#vtkPolyDataWriter VTKW

    # set the random seed
	k setRandSeed           $i

    # could equally well play with other stuff
#    bsr setRadius [expr $i * 0.1 + 2.0]
#    bsr setCentre "[expr $i * 1.0 + 10.0] 42 10"
    k setSource bsr

	k startAsync

    progresstimer

	k awaitFinish

    # get results
#   fluencerep Update [k getSurfaceFluenceVector] 1

    # write the file out
#    W writeSurfaceFluence "fluence.$i.txt" [k getSurfaceFluenceVector]

#    VTKW SetHeader "(header comment)"
#	VTKW SetFileName "surface.vtk"
#	VTKW SetInput [fluencerep getPolyData]
#	VTKW Update
