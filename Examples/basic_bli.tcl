###### Start performance timer
set tStart [clock clicks -milliseconds]

###### Import VTK TCL package
package require vtk




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




###### Basic parameters: file names

#default file prefix
set pfx "/usr/local/share/fullmontesw/examples/mousebli/mouse"


#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "$pfx.opt"
set meshfn "$pfx.mesh"




###### Read problem definition in TIMOS format

TIMOSAntlrParser R

R setMeshFileName $meshfn
R setOpticalFileName $optfn

set mesh [R mesh]
set opt [R materials_simple]


set tLoaded [clock clicks -milliseconds]


###### Configure source (Ball source with given centre and radius)

Ball B
B centre "10 40 11"
B radius 2





###### Create and configure simulation kernel with surface scoring

TetraSurfaceKernel k $mesh

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

k packetCount        1000000
    # number of packets to simulate (more -> better quality, longer run)

k threadCount        8
    # number of threads (set to number of cores, or 2x number of cores if hyperthreading)






###### VTK output pipeline

# wrap FullMonte mesh and make available as vtkPolyData & vtkUnstructuredGrid
vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh $mesh

# Create fluence wrapper
vtkFullMonteArrayAdaptor vtkPhi

vtkFieldData surfaceFieldData

vtkDataObject surfaceData
    surfaceData SetFieldData surfaceFieldData

# merge fluence wrapper onto the surface triangles of the mesh 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData surfaceData
    mergeFluence SetInputData [VTKM faces]
    mergeFluence SetOutputFieldToCellDataField

# select out only the surface triangles (no fluence scored for interior triangles in this kernel)
TriFilterRegionBounds TF
    TF mesh $mesh
    TF bidirectional 1
    TF includeRegion 0 1

# create an ID list from the filter
vtkFullMonteFilterTovtkIdList surfaceTriIDs 
    surfaceTriIDs mesh $mesh
    surfaceTriIDs filter [TF self]

# extract the faces with listed IDs
vtkExtractCells extractSurface
    extractSurface SetInputConnection [mergeFluence GetOutputPort]
    extractSurface SetCellList [surfaceTriIDs idList]

# vtkExtractCells produces a vtkUnstructuredGrid; input is vtkPolyData so output will be too
#   vtkGeometryFilter will extract the surface triangles from it
vtkGeometryFilter geom
    geom SetInputConnection [extractSurface GetOutputPort]

# set up writer for the output poly data
vtkPolyDataWriter VTKW
    VTKW SetInputConnection [geom GetOutputPort]





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





###### Run N experiments with different source placements

set N 5

for { set i 0 } { $i < $N } { incr i } {

    puts "Trial $i of $N"

    # set the random seed
	k randSeed           $i

    # arbitrarily perturb the radius and centre of the ball source
    B radius [expr $i * 0.1 + 2.0]
    B centre "[expr $i * 1.0 + 10.0] 42 10"
	k source B

    # launch kernel, display progress timer, and await finish

	k startAsync
    progresstimer
	k finishAsync

    puts "Kernel is done"

    OutputDataSummarize OS

    for { set r 0 } { $r < [k getResultCount] } { incr r } {
        puts ""
        puts "Result $r"
        puts "  [k getResultByIndex $r] [[k getResultByIndex $r] typeString]"
        OS visit [k getResultByIndex $r]
    }

    puts "** end summary **"

    vtkPhi source [k getResultByIndex 2]
    vtkPhi update
    set A [vtkPhi array]

    surfaceFieldData AddArray $A

    # Write the surface fluence data out to a .vtk file (binary format)
    #   updating the fluence map's VTK adaptor (vtkPhi) will ripple through the VTK pipeline when Update is called

    VTKW SetFileName "mouse.$i.surf.vtk"
    VTKW SetFileTypeToBinary
    VTKW Update

    surfaceFieldData RemoveArray $A
}
