###### Import VTK TCL package
package require vtk

puts "loaded vtk"


###### Load required FullMonte libraries

# TIMOS format reader (for Digimouse) with TCL bindings
load libFullMonteTIMOSTCL.so

# Geometry model with TCL bindings
load libFullMonteGeometryTCL.so

# Software kernels with TCL bindings
load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so

# Data output manipulation
load libFullMonteDataTCL.so

# VTK interface
load libFullMonteVTKTCL.so


puts "loaded libs"



###### Basic parameters: file names

#default file prefix
set pfx "/Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse"


#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "$pfx.opt"
set meshfn "$pfx.mesh"
set legendfn "$pfx.legend"
set ofn "fluence.out"




###### Read problem definition in TIMOS format

TIMOSAntlrParser R

R setMeshFileName $meshfn
R setOpticalFileName $optfn
R setLegendFileName $legendfn

set mesh [R mesh]
set opt [R materials_simple]




###### Configure source

Ball B
B centre "10 40 11"
B radius 2





###### Create and configure simulation kernel with surface scoring

TetraTraceKernel k $mesh

k source B
    # the source to launch from

k energy             50
    # total energy

k materials          $opt
    # materials

k setUnitsToMM
    # units for mesh dimensions & optical properties (must match each other)



# Monte Carlo kernel properties
k roulettePrWin      0.1
    # probability of roulette win

k rouletteWMin       1e-5
    # minimum weight "wmin" before roulette takes effect

k maxSteps           10000
    # maximum number of steps to trace a packet

k maxHits            100
    # maximum number of boundaries a single step can take

k packetCount        1024
    # number of packets to simulate (more -> better quality, longer run)

k threadCount        8
    # number of threads (set to number of cores, or 2x number of cores if hyperthreading)





###### Define progress timer callback function for use during simulation run

proc progresstimer {} {
    # loop while not finished
	while { ![k done] } {
        # display % completed to 2 decimal places
    	puts -nonewline [format "\rProgress %6.2f%%" [expr 100.0*[k progressFraction]]]
	    flush stdout

        # refresh interval: 200ms
	    after 200
	}
	puts [format "\rProgress %6.2f%%" 100.0]
}





##### Run it

    # launch kernel, display progress timer, and await finish

	k startAsync
    progresstimer
	k finishAsync

for { set i 0 } { $i < [k getResultCount] } { incr i } { puts "  [[k getResultByIndex $i] typeString]" }

set paths [k getResultByTypeString "PacketPositionTraceSet"]

puts "paths=$paths"
#puts "Returned a path with [$paths nTraces] traces and a total of [$paths nPoints] points"

vtkFullMontePacketPositionTraceSetToPolyData O
    O source $paths
    O update


vtkPolyDataWriter W
    W SetInputData [O getPolyData]
    W SetFileName "traces.vtk"
    W Update
    W Delete
