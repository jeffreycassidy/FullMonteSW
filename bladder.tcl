package require vtk

load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteMatlabTCL.so

load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so
load libFullMonteVTKFileTCL.so
load libFullMonteVTKTCL.dylib

#default file prefix
set meshfn "/Users/jcassidy/src/FullMonteSW/FullMonteSW/bladder\&rectum\&prostate.mesh.vtk"
set pfx "/Users/jcassidy/src/FullMonteSW/FullMonteSW/bladder"


#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "$pfx.opt"
#set meshfn "$meshfn"
set legendfn "$pfx.legend"
set ofn "fluence.out"

# create and set up the reader

TIMOSAntlrParser R
VTKLegacyReader VTKR

VTKR addZeroPoint 1
VTKR addZeroCell 1

VTKR setFileName $meshfn
R setOpticalFileName $optfn
#R setLegendFileName $legendfn


set mesh [VTKR mesh]
set opt [R materials_simple]

set P [PointSource_New]
$P position "-8.6 49.7 -1418.70"

## Create sim kernel

TetraMesh M $mesh

TetraVolumeKernel k M


puts "mesh points: [$mesh getNp]"

puts "M Points: [M getNp]"


# Kernel properties
k source $P
k energy             50
k materials          $opt
k setUnitsToMM


# Monte Carlo kernel properties
k roulettePrWin      0.1
k rouletteWMin       1e-5
k maxSteps           10000
k maxHits            10000
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

set N 1

## Wrap the TetraMesh into VTK unstructured grid

vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh [M self]

## Create a filter to wrap the fluence data into a vtkFloatArray
vtkFullMonteSpatialMapWrapperFU VTKPhi

set volumeFluenceArray [VTKPhi array]
    $volumeFluenceArray SetName "Volume Fluence (au)"

set regionArray [VTKM regions]
    $regionArray SetName "Tissue type"

vtkFieldData volumeFieldData
    volumeFieldData AddArray $volumeFluenceArray
    volumeFieldData AddArray $regionArray

vtkDataObject volumeData
    volumeData SetFieldData volumeFieldData
 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData volumeData
    mergeFluence SetInputData [VTKM blankMesh]
    mergeFluence SetOutputFieldToCellDataField

vtkUnstructuredGridWriter VTKW
    VTKW SetInputConnection [mergeFluence GetOutputPort]


for { set i 0 } { $i < $N } { incr i } {

    puts "Trial $i of $N"
    
    # set the random seed
	k randSeed           $i

    # could equally well play with other stuff
#    $P position "[expr $i * 1.0 + 10.0] 42 10"
#	k source $P

    puts "[$P position]"

	k startAsync

    progresstimer

	k finishAsync

    set foo [k getVolumeFluenceMap]

    VTKPhi source $foo
    VTKPhi update
    mergeFluence Update

    # write as .vtk unstructured grid
    VTKW SetFileName "fluence.$i.bin.vtk"
    VTKW SetFileTypeToBinary
    VTKW Update
}
