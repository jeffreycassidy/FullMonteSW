###### Load libraries

# Load the VTK package for visualization
package require vtk

# Load the necessary FullMonte libraries
load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteMatlabTCL.so
load libFullMonteDataTCL.so

load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so
load libFullMonteVTKTCL.dylib
load libFullMonteVTKFileTCL.so



###### Set file names and load files

set meshfn "/Users/jcassidy/src/FullMonteSW/FullMonteSW/bladder&rectum&prostate.mesh.vtk"
set optfn  "/Users/jcassidy/src/FullMonteSW/FullMonteSW/bladder.opt"


# create and set up the mesh reader - file is saved as a VTK "legacy" .vtk file (directly loadable in VTK/Paraview)
# there is a scalar field denoting tissue type 
VTKLegacyReader VTKR
    VTKR setFileName $meshfn
    VTKR addZeroPoint 1
    VTKR addZeroCell 1

set MB [VTKR mesh]

# read optical properties in TIM-OS format text file (described at https://sites.google.com/site/haioushen/software)
TIMOSAntlrParser R
    R setOpticalFileName $optfn

set opt [R materials_simple]



###### Instantiate and configure simulation kernel (SV kernel -> score Surface & Volume)

TetraSVKernel k $mesh

# Create and place source (units are mm, referenced to the input geometry)
# This placement was derived by loading the mesh in Paraview and manipulating a point source
Point P
P position "-8.6 49.7 -1418.70"

# Kernel properties
    k source $P
    k energy             50
    k materials          $opt
    k setUnitsToMM

# Monte Carlo kernel properties (standard, unlikely to need change)
    k roulettePrWin      0.1
    k rouletteWMin       1e-5
    k maxSteps           10000
    k maxHits            10000

# Number of packets to simulate. More -> higher quality, longer run time. Try 10^6 to start.
    k packetCount        1000000

# Thread count should be number of cores on the machine, or 2x number of cores with SMT (aka. "Hyperthreading")
    k threadCount        8


proc progresstimer {} {
	while { ![k done] } {
	puts -nonewline [format "\rProgress %6.2f%%" [expr 100.0*[k progressFraction]]]
	flush stdout
	after 200
	}
	puts [format "\rProgress %6.2f%%" 100.0]
}



###### Select the internal surfaces for fluence counting

# bidirectional=1 -> count fluence entering and exiting
TriFilterRegionBounds TF
    TF mesh $mesh
    TF bidirectional 1
    TF includeRegion 1 1

$mesh setFacesForFluenceCounting TF



###### VTK volume data visualization pipeline

# Make FullMonte mesh data structure available to VTK
vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh $mesh

# vtkFieldData stores multiple named arrays which hold one entry per geometry element (in this case tetrahedron)
vtkFieldData volumeFieldData
    volumeFieldData AddArray [VTKM regions]

# data object holds the field data
vtkDataObject volumeDataObject
    volumeDataObject SetFieldData volumeFieldData



# Set up VTK filter to merge the provided data (region & fluence) onto the provided mesh
vtkMergeDataObjectFilter mergeVolume
    mergeVolume SetDataObjectInputData volumeDataObject
    mergeVolume SetInputData [VTKM blankMesh]
    mergeVolume SetOutputFieldToCellDataField

# Set up VTK writer to write the resulting volume data to a file
vtkUnstructuredGridWriter VW
    VW SetInputConnection [mergeVolume GetOutputPort]
    VW SetFileName "bladder.volume.vtk"



###### VTK surface data visualization pipeline



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
