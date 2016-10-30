package require vtk

load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteMatlabTCL.so
load libFullMonteDataTCL.so

load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so
load libFullMonteVTKTCL.so

#default file prefix
set pfx "/Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse"


#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set optfn "$pfx.opt"
set meshfn "$pfx.mesh"
set legendfn "$pfx.legend"

# create and set up the reader

TIMOSAntlrParser R

R setMeshFileName $meshfn
R setOpticalFileName $optfn
R setLegendFileName $legendfn


set mesh [R mesh]
set opt [R materials_simple]

Ball B
B centre "10 40 11"
B radius 2

## Create sim kernel

TetraSVKernel k $mesh

# Kernel properties
k source B
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

set N 1


# Set up internal fluence counting

TriFilterRegionBounds TF
    TF mesh $mesh
    TF bidirectional 1

# designate regions whose boundaries should be monitored
    TF includeRegion 12 1
    TF includeRegion 3  1


TetraFilterByRegion MF
    MF mesh $mesh
    MF include 12 1



$mesh setFacesForFluenceCounting TF

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

vtkUnstructuredGridWriter VW
    VW SetInputConnection [mergeVolume GetOutputPort]
    VW SetFileName "mouse.volume.vtk"



## Writer pipeline for surface field data (organ-surface fluence)

vtkFullMonteSpatialMapWrapperFU vtkPhi

set surfaceFluenceArray [vtkPhi array]
    $surfaceFluenceArray SetName "Surface Fluence (au)"

vtkFieldData surfaceFieldData
    surfaceFieldData AddArray $surfaceFluenceArray

puts "surfaceFieldData size: [surfaceFieldData GetNumberOfTuples]"
puts "number of faces:       [[VTKM faces] GetNumberOfCells]"

vtkDataObject surfaceData
    surfaceData SetFieldData surfaceFieldData
 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData surfaceData
    mergeFluence SetInputData [VTKM faces]
    mergeFluence SetOutputFieldToCellDataField

vtkFullMonteFilterTovtkIdList surfaceTriIDs 
    surfaceTriIDs mesh $mesh
    surfaceTriIDs filter [TF self]


vtkExtractCells extractSurface
    extractSurface SetInputConnection [mergeFluence GetOutputPort]
    extractSurface SetCellList [surfaceTriIDs idList]

vtkGeometryFilter geom
    geom SetInputConnection [extractSurface GetOutputPort]

vtkPolyDataWriter VTKW
    VTKW SetInputConnection [geom GetOutputPort]

DoseSurfaceHistogramGenerator DSHG
    DSHG mesh $mesh
    DSHG filter TF

DoseVolumeHistogramGenerator DVHG
    DVHG mesh $mesh
    DVHG filter MF

BidirectionalFluence BF

FluenceConverter FC
    FC mesh $mesh
    FC materials $opt

for { set i 0 } { $i < $N } { incr i } {

    puts "Trial $i of $N"
    
    # set the random seed
	k randSeed           $i

    # could equally well play with other stuff
    B radius [expr $i * 0.1 + 2.0]
    B centre "[expr $i * 1.0 + 10.0] 42 10"
	k source B

	k startAsync

    progresstimer

	k finishAsync


    BF source [k getInternalSurfaceFluenceMap]

    set Emap [FC convertToEnergyDensity [k getVolumeAbsorbedEnergyMap]]

    set phiV [FC convertToFluence [k getVolumeAbsorbedEnergyMap]]

    vtkFullMonteArrayAdaptor EmapAdaptor
        EmapAdaptor source $Emap

    vtkFullMonteArrayAdaptor PhiAdaptor
        PhiAdaptor source [k getVolumeFluenceMap]

    puts "Emap = $Emap"

    set phi [BF result]

    vtkPhi source $phi
    vtkPhi update


    DSHG fluence $phi
    set dsh [DSHG result]

    puts "DSH generated"

    volumeFieldData AddArray [EmapAdaptor result]
    volumeFieldData AddArray [PhiAdaptor result]

    $dsh print

    DVHG fluence $phiV
    set dvh [DVHG result]

    puts "DVH generated"

    $dvh print

    VTKW SetFileName "internalsurface.vtk"
    VTKW Update

    VW Update
}
