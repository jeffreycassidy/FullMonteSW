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
load libFullMonteQueriesTCL.so

# VTK interface
load libFullMonteVTKTCL.so




###### Basic parameters: file names

#default file prefix
set pfx "/usr/local/share/fullmontesw/examples/mousebli/mouse"


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

Line L



###### Create and configure simulation kernel with volume scoring

TetraVolumeKernel k $mesh

k source L
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



EnergyToFluence EF
    EF mesh $mesh
    EF materials $opt

vtkFullMonteArrayAdaptor vtkPhi

###### VTK visualization & result export

vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh $mesh

## Writer pipeline for volume field data (regions & vol fluence)

vtkFieldData volumeFieldData
    volumeFieldData AddArray [VTKM regions]

vtkDataObject volumeDataObject
    volumeDataObject SetFieldData volumeFieldData

vtkMergeDataObjectFilter mergeVolume
    mergeVolume SetDataObjectInputData volumeDataObject
    mergeVolume SetInputData [VTKM blankMesh]
    mergeVolume SetOutputFieldToCellDataField

vtkUnstructuredGridWriter W
    W SetInputConnection [mergeVolume GetOutputPort]

#DoseHistogramGenerator DVHG
#    DVHG mesh $mesh
#    DVHG filter TF



vtkSourceExporter SE


vtkUnstructuredGridWriter SW



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






## Bounding box
#x [7,17]
#y [35.5, 45.5]
#z [6.5 16.5]

proc runsim { case } { 
    ## Set up source location
    
    if { $case == "good" } {
        puts "good case" 

        # good 
        L endpoint 0 "11.4 42.4 10.4"
        L endpoint 1 "11.7 41.6 11.0"
    } elseif { $case == "bad" } {
        puts "bad case"

        # bad
        L endpoint 0 "11.5 40.0 10.4"
        L endpoint 1 "12.5 45.1 12.1"
    } else {
        puts "ERROR: Invalid"
    }

    # launch kernel, display progress timer, and await finish
	k startAsync
    progresstimer
	k finishAsync

    set volume [k getResultByTypeString "SpatialMap"]

    puts "volume=$volume"

    EF source $volume
    EF update

    vtkPhi source [EF result]
    vtkPhi update

    volumeFieldData AddArray [vtkPhi array]

    W SetFileName "mouse.$case.vtk"

    volumeFieldData RemoveArray [vtkPhi array]

    W Update


    ## Write out sources
    SW SetFileName "mouse.sources.$case.vtk"
    SE source L
    SW SetInputData [SE output]
    SW Update
}


###### Actually run the sims

runsim good
runsim bad

