###### Load libraries

# Load the VTK package for visualization
package require vtk

# Stop Tk from popping up an empty window
wm withdraw .

# Load the necessary FullMonte libraries
load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so
load libFullMonteMatlabTCL.so
load libFullMonteDataTCL.so

load libFullMonteSWKernelTCL.so
load libFullMonteKernelsTCL.so
load libFullMonteQueriesTCL.so
load libFullMonteVTKTCL.so
load libFullMonteVTKFileTCL.so


###### Set file names and load files
set datapath "/Users/jcassidy/src/FullMonteSW/FullMonteSW/data/bladder"
set meshfn "$datapath/bladder&rectum&prostate.mesh.vtk"
set opt_clear  "$datapath/bladder_clear.opt"
set opt_intra "$datapath/bladder_intralipid.opt"

# create and set up the mesh reader - file is saved as a VTK "legacy" .vtk file (directly loadable in VTK/Paraview)
# there is a scalar field denoting tissue type 
VTKLegacyReader VTKR
    VTKR setFileName $meshfn
    VTKR addZeroPoint 1
    VTKR addZeroCell 1

set MB [VTKR mesh]
set mesh [TetraMesh foo $MB]

# read optical properties in TIM-OS format text file (described at https://sites.google.com/site/haioushen/software)
TIMOSAntlrParser R
    R setOpticalFileName $opt_clear



###### Instantiate and configure simulation kernel (SV kernel -> score Surface & Volume)

TetraSVKernel k $mesh

# Create and place source (units are mm, referenced to the input geometry)
# This placement was derived by loading the mesh in Paraview and manipulating a point source
Point P
P position "-8.6 49.7 -1418.70"

# Kernel properties
    k source P
    k energy             50
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
    # loop until kernel is done
	while { ![k done] } {
    	puts -nonewline [format "\rProgress %6.2f%%" [expr 100.0*[k progressFraction]]]
	    flush stdout

        # wait 200ms
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

TetraFilterByRegion TETF
	TETF mesh $mesh
	TETF excludeAll
	TETF include 1 1

$mesh setFacesForFluenceCounting TF



###### Output conversion and processing

# Convert energy exiting through each face to fluence (divide by area)
EnergyToFluence EFS
	EFS mesh $mesh

# Convert energy absorbed in each tetra to fluence (divide by area*mu_a)
EnergyToFluence EFV
	EFV mesh $mesh

EnergyToFluence EFDS
	EFDS mesh $mesh

# sum the entering and exiting fluence at each surface to get bidirectional fluence
DirectionalSurface DS
	DS mesh $mesh
	DS direction 2
	DS tetraFilter TETF

DirectionalSurface BI
	BI mesh $mesh
	BI direction 3
	BI tetraFilter TETF

#DoseSurfaceHistogramGenerator DSHG
#    DSHG mesh $mesh
#    DSHG filter TF



###### VTK file write pipeline

# Make FullMonte mesh data structure available to VTK
vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh $mesh


### VOLUME DATA

vtkFullMonteArrayAdaptor vtkPhiV

# vtkFieldData stores multiple named arrays which hold one entry per geometry element (here, tetrahedron)
# first add an array "Tissue Type" from the mesh regions
# we will later add fluence for each tetrahedral volume element
vtkFieldData volumeFieldData
    volumeFieldData AddArray [VTKM regions]

# data object holds the field data
vtkDataObject volumeDataObject
    volumeDataObject SetFieldData volumeFieldData

# Merge the mesh geometry with the field data object
vtkMergeDataObjectFilter mergeVolume
    mergeVolume SetDataObjectInputData volumeDataObject
    mergeVolume SetInputData [VTKM blankMesh]
    mergeVolume SetOutputFieldToCellDataField

# write the resulting volume data to a file
vtkUnstructuredGridWriter VW
    VW SetInputConnection [mergeVolume GetOutputPort]
    VW SetFileName "bladder.volume.vtk"



### SURFACE DATA

vtkFullMonteArrayAdaptor vtkPhiS

vtkFullMonteArrayAdaptor vtkPhiDS

vtkFieldData surfaceFieldData

vtkDataObject surfaceData
    surfaceData SetFieldData surfaceFieldData
 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData surfaceData
    mergeFluence SetInputData [VTKM faces]
    mergeFluence SetOutputFieldToCellDataField

# filter out only the triangles that were selected
vtkFullMonteFilterTovtkIdList surfaceTriIDs 
    surfaceTriIDs mesh $mesh
    surfaceTriIDs filter [TF self]

vtkExtractCells extractSurface
    extractSurface SetInputConnection [mergeFluence GetOutputPort]
    extractSurface SetCellList [surfaceTriIDs idList]

# convert vtkUnstructuredGrid output of extractcells (has only triangles anyway) to vtkPolyData
vtkGeometryFilter geom
    geom SetInputConnection [extractSurface GetOutputPort]

vtkPolyDataWriter VTKW
    VTKW SetInputConnection [geom GetOutputPort]


foreach case { bladder_clear bladder_intralipid } {

	set optfn $datapath/$case.opt
puts "$optfn"

# load the appropriate optical properties
	R setOpticalFileName $optfn
	set opt [R materials_simple]
	k materials $opt

	EFV materials $opt

# start the kernel, show the progress timer, and wait until the kernel is done
	k startAsync
    progresstimer
	k finishAsync

# update volume fluence converted with volume absorption data, write to file
	EFV source [k getResultByIndex 2]
	EFV update

	vtkPhiV source [EFV result]
	volumeFieldData AddArray [vtkPhiV array]

	VW SetFileName "$case.volume.bin.vtk"
	VW SetFileTypeToBinary
	VW Update

# update surface fluence conversion with surface exit data, write to file
	DS data [k getResultByIndex 4]
	DS update

	EFS source [DS result]
	EFS update

	vtkPhiS source [EFS result]
	surfaceFieldData AddArray [vtkPhiS array]

	puts "EFS done"

# add directed surface fluence
	EFDS source [k getResultByIndex 4]
	EFDS update

	BI data [EFDS result]
	BI update

	vtkPhiDS source [BI result]
	surfaceFieldData AddArray [vtkPhiDS array]

#    DSHG fluence $phi
#    set dsh [DSHG result]

#    $dsh print

    VTKW SetFileName "$case.surface.bin.vtk"
#    VTKW SetFileTypeToBinary
    VTKW Update

}
