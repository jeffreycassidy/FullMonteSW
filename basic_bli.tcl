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


## Wrap the TetraMesh into VTK unstructured grid

vtkFullMonteTetraMeshWrapper VTKM
    VTKM mesh $mesh

## Create a filter to wrap the fluence data into a vtkFloatArray
vtkFullMonteSpatialMapWrapperFU VTKPhi

set surfaceFluenceArray [VTKPhi array]
    $surfaceFluenceArray SetName "Surface Fluence (au)"

vtkFieldData surfaceFieldData
    surfaceFieldData AddArray $surfaceFluenceArray

vtkDataObject surfaceData
    surfaceData SetFieldData surfaceFieldData
 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData surfaceData
    mergeFluence SetInputData [VTKM faces]
    mergeFluence SetOutputFieldToCellDataField


## Create a filter to wrap the _internal_ fluence data into a vtkFloatArray
vtkFullMonteSpatialMapWrapperFU VTKPhiInt

set intSurfaceFluenceArray [VTKPhi array]
    $intSurfaceFluenceArray SetName "Surface Fluence (au)"

vtkFieldData surfaceFieldData
    surfaceFieldData AddArray $intSurfaceFluenceArray

vtkDataObject surfaceData
    surfaceData SetFieldData surfaceFieldData
 
vtkMergeDataObjectFilter mergeFluence
    mergeFluence SetDataObjectInputData surfaceData
    mergeFluence SetInputData [VTKM faces]
    mergeFluence SetOutputFieldToCellDataField


### Set up internal fluence vis

set intFp [TriFilterRegionBounds F]
    F mesh $mesh
    F bidirectional 1
    F includeRegion  9 1
    F includeRegion 10 1
    F includeRegion 12 1

vtkExtractCells internalSubset
    internalSubset SetCellList [VTKM getTriangleIDsFromFilter $intFp]
    internalSubset SetInputConnection [mergeFluence GetOutputPort]

vtkGeometryFilter geomInt
    geomInt SetInputConnection [internalSubset GetOutputPort]


vtkPolyDataWriter VTKWInt
    VTKWInt SetInputConnection [geomInt GetOutputPort]


### Set up surface fluence vis

set surfFp [TriFilterRegionBounds surfF]
    surfF mesh $mesh
    surfF bidirectional 1
    surfF includeRegion 0 1

vtkExtractCells surfaceSubset
    surfaceSubset SetCellList [VTKM getTriangleIDsFromFilter $surfFp]
    surfaceSubset SetInputConnection [mergeFluence GetOutputPort]

vtkGeometryFilter geomSurf
    geomSurf SetInputConnection [surfaceSubset GetOutputPort]

vtkPolyDataWriter VTKWSurf
    VTKWSurf SetInputConnection [geomSurf GetOutputPort]


set B [Ball_New]
$B centre "10 40 11"
$B radius 2

## Create sim kernel

TetraSVKernel k $mesh
    $mesh setFacesForFluenceCounting $intFp


# Kernel properties
k source $B
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

#MatlabWriter W
#TextFileWriter W
#W mesh $mesh
#set surf [$mesh getRegionBoundaryTris 0]
#W threshold 0
#W faceSubset $surf
#W writeFaces "faces.txt"

for { set i 0 } { $i < 1 } { incr i } {

    puts "Trial $i of $N"
    
    # set the random seed
	k randSeed           $i

    # could equally well play with other stuff
    $B radius [expr $i * 0.1 + 2.0]
    $B centre "[expr $i * 1.0 + 10.0] 42 10"
	k source $B

	k startAsync

    progresstimer

	k finishAsync

    # get results
 #   fluencerep Update [k getSurfaceFluenceVector] 1

    # write the file out
#    W comment "Experiment run $i with blah blah blah"
#	W writeSurfaceFluence "fluence.expt$i.txt" [k getSurfaceFluence]
    VTKPhi source [k getSurfaceFluenceMap]
    VTKPhi update


    VTKWInt SetFileName "fluence.int.$i.vtk"
    VTKWInt Update

    VTKWSurf SetFileName "fluence.surf.$i.vtk"
    VTKWSurf Update
}
