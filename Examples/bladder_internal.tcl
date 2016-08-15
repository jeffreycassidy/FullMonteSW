package require vtk

load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteMatlabTCL.so
load libFullMonteDataTCL.so

load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so
load libFullMonteVTKTCL.dylib
load libFullMonteVTKFileTCL.so

#default file prefix
set meshfn "/Users/jcassidy/src/FullMonteSW/FullMonteSW/bladder&rectum&prostate.mesh.vtk"
set optfn  "/Users/jcassidy/src/FullMonteSW/FullMonteSW/bladder.opt"

#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

# create and set up the reader

VTKLegacyReader VTKR
    VTKR setFileName $meshfn
    VTKR addZeroPoint 1
    VTKR addZeroCell 1

TIMOSAntlrParser R

R setOpticalFileName $optfn


set MB [VTKR mesh]

set mesh [TetraMesh foo $MB]

set opt [R materials_simple]

set P [PointSource_New]
$P position "-8.6 49.7 -1418.70"

## Create sim kernel

TetraSVKernel k $mesh

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


# Set up internal fluence counting

TriFilterRegionBounds TF
    TF mesh $mesh
    TF bidirectional 1

# designate regions whose boundaries should be monitored
    TF includeRegion 1 1



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
    VW SetFileName "bladder.volume.vtk"



vtkFullMonteSpatialMapWrapperFU vtkPhi

set surfaceFluenceArray [vtkPhi array]
    $surfaceFluenceArray SetName "Surface Fluence (au)"

vtkFieldData surfaceFieldData
    surfaceFieldData AddArray $surfaceFluenceArray

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

BidirectionalFluence BF

FluenceConverter FC
    FC mesh $mesh

for { set i 0 } { $i < $N } { incr i } {

    puts "Trial $i of $N"
    
    # set the random seed
	k randSeed           $i

	k source $P

	k startAsync

    progresstimer

	k finishAsync

    set Emap [FC convertToEnergyDensity [k getVolumeAbsorbedEnergyMap]]

    vtkFullMonteArrayAdaptor EmapAdaptor
        EmapAdaptor source $Emap

    vtkFullMonteArrayAdaptor PhiAdaptor
        PhiAdaptor source [k getVolumeFluenceMap]

    puts "Simulation finished"

    BF source [k getInternalSurfaceFluenceMap]

    set phi [BF result]
    vtkPhi source $phi
    vtkPhi update

    puts "vtkPhi update done"

    geom Update

    DSHG fluence $phi
    set dsh [DSHG result]

    $dsh print

    puts "Geom update done"

    VTKW SetFileName "internalsurface.bin.vtk"
    VTKW SetFileTypeToBinary
    VTKW Update


    volumeFieldData AddArray [EmapAdaptor result]
    volumeFieldData AddArray [PhiAdaptor result]

    VW Update
}
