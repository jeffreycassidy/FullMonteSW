package require vtk

load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteMatlabTCL.so

load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so
load libFullMonteVTKTCL.dylib

#default file prefix
set pfx "/Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse"


#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "$pfx.opt"
set meshfn "$pfx.mesh"
set legendfn "$pfx.legend"
set ofn "fluence.out"

# create and set up the reader

TIMOSAntlrParser R

R setMeshFileName $meshfn
R setOpticalFileName $optfn
R setLegendFileName $legendfn


set mesh [R mesh]
set opt [R materials_simple]

set P [PointSource_New]
$P position "10 40 11"

## Create sim kernel

TetraVolumeKernel k $mesh


# Kernel properties
k source $P
k energy             50
k materials          $opt
k setUnitsToMM


# Monte Carlo kernel properties
k roulettePrWin      0.1
k rouletteWMin       1e-5
k maxSteps           10000
k maxHits            100
k packetCount        1000000
k threadCount        8


proc progresstimer {} {
	while { ![k done] } {
	puts -nonewline [format "\rProgress %6.2f%%" [expr 100.0*[k progressFraction]]]
	flush stdout
	after 200
	}
	puts [format "\rProgress %6.2f%%" 100.0]
}

set N 9
#VTKSurfaceFluenceRep fluencerep V
#vtkPolyDataWriter W

# Choose the surface to be output
set surf [$mesh getRegionBoundaryTris 0]



#MatlabWriter W
##TextFileWriter W
#W mesh $mesh
#W threshold 0
#W faceSubset $surf
#W writeFaces "faces.txt"

## Wrap the TetraMesh into VTK unstructured grid

vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh $mesh

## Create a filter to wrap the fluence data into a vtkFloatArray
vtkFullMonteSpatialMapWrapperFU VTKPhi

set volumeFluenceArray [VTKPhi array]
    $volumeFluenceArray SetName "Volume Fluence (au)"

vtkFieldData volumeFieldData
    volumeFieldData AddArray $volumeFluenceArray

vtkDataObject volumeData
    volumeData SetFieldData volumeFieldData
 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData volumeData
    mergeFluence SetInputData [VTKM blankMesh]
    mergeFluence SetOutputFieldToCellDataField

vtkUnstructuredGridWriter VTKW
    VTKW SetInputConnection [mergeFluence GetOutputPort]


for { set i 0 } { $i < 1 } { incr i } {

    puts "Trial $i of $N"
    
    # set the random seed
	k randSeed           $i

    # could equally well play with other stuff
    $P position "[expr $i * 1.0 + 10.0] 42 10"
	k source $P

	k startAsync

    progresstimer

	k finishAsync

    # get results
 #   fluencerep Update [k getSurfaceFluenceVector] 1

    # write as Matlab file
#    W comment "Experiment run $i with blah blah blah"
#	W writeSurfaceFluence "fluence.expt$i.txt" [k getSurfaceFluence]

    VTKPhi source [k getVolumeFluence]
    VTKPhi update
    mergeFluence Update

    # write as .vtk unstructured grid
    VTKW SetFileName "fluence.$i.vtk"
    VTKW Update
}
