###### Start performance timer
set tStart [clock clicks -milliseconds]

###### Import VTK TCL package
package require vtk
wm withdraw .




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
#set path "/Users/jcassidy/src/FullMonteSW/data"
set path "/home/jcassidy/src/FullMonteSW/data/TIM-OS"





###### Read problem definition in TIMOS format

TIMOSAntlrParser R





###### Create and configure simulation kernel with surface scoring

TetraVolumeKernel k


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

k packetCount        10000000
    # number of packets to simulate (more -> better quality, longer run)

k threadCount        8
    # number of threads (set to number of cores, or 2x number of cores if hyperthreading)






###### VTK output pipeline

# wrap FullMonte mesh and make available as vtkPolyData & vtkUnstructuredGrid
vtkFullMonteTetraMeshWrapper VTKM

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

# create an ID list from the filter
#vtkFullMonteFilterTovtkIdList surfaceTriIDs 
#    surfaceTriIDs mesh $mesh
#    surfaceTriIDs filter [TF self]

# extract the faces with listed IDs
#vtkExtractCells extractSurface
#    extractSurface SetInputConnection [mergeFluence GetOutputPort]
#    extractSurface SetCellList [surfaceTriIDs idList]

# vtkExtractCells produces a vtkUnstructuredGrid; input is vtkPolyData so output will be too
#   vtkGeometryFilter will extract the surface triangles from it
#vtkGeometryFilter geom
#    geom SetInputConnection [extractSurface GetOutputPort]

# set up writer for the output poly data
#vtkPolyDataWriter VTKW
#    VTKW SetInputConnection [geom GetOutputPort]





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

proc loadsource { srcfn } {
    global src

    R setSourceFileName $srcfn
    
    set src [R sources]
    set tStart [clock clicks -milliseconds]

    k source $src

    set tSimEnd [clock clicks -milliseconds]
    puts "  Source load from $srcfn: [expr ($tSimEnd-$tStart)*1e-3] s"
}

proc loadmaterials { optfn } {
    global opt
    set tStart [clock clicks -milliseconds]

    R setOpticalFileName $optfn

    set opt [R materials_simple]

    k materials $opt

    set tSimEnd [clock clicks -milliseconds]
    puts "  Materials load from $optfn: [expr ($tSimEnd-$tStart)*1e-3] s"
}

proc loadmesh { meshfn } { 
    set tStart [clock clicks -milliseconds]
    global mesh

    R setMeshFileName $meshfn

    set mesh [R mesh]

    k mesh $mesh

    VTKM mesh $mesh

    set tSimEnd [clock clicks -milliseconds]
    puts "  Mesh load from $meshfn: [expr ($tSimEnd-$tStart)*1e-3] s"
}



proc runcase { Npkt outname } { 

    k packetCount $Npkt

    # launch kernel, display progress timer, and await finish
    set tSimStart [clock clicks -milliseconds]

	k startAsync
    progresstimer
	k finishAsync
    set tSimEnd [clock clicks -milliseconds]

    OutputDataSummarize OS

    for { set r 0 } { $r < [k getResultCount] } { incr r } {
        puts ""
        puts "Result $r"
        puts "  [k getResultByIndex $r] [[k getResultByIndex $r] typeString]"
        OS visit [k getResultByIndex $r]
    }

    vtkPhi source [k getResultByIndex 2]
    vtkPhi update
    set A [vtkPhi array]

    surfaceFieldData AddArray $A

    # Write the surface fluence data out to a .vtk file (binary format)
    #   updating the fluence map's VTK adaptor (vtkPhi) will ripple through the VTK pipeline when Update is called

#    VTKW SetFileName "$outname.surf.vtk"
#    VTKW SetFileTypeToBinary
#    VTKW Update

    surfaceFieldData RemoveArray $A

    puts "  Sim time: [expr ($tSimEnd-$tSimStart)*1e-3]"
}


loadmaterials "$path/mouse/mouse.opt"
loadmesh "$path/mouse/mouse.mesh"
loadsource "$path/mouse/mouse.source"

runcase 10000000 "mouse"

loadmesh "$path/cube_5med/cube_5med.mesh"
loadmaterials "$path/cube_5med/cube_5med.opt"
loadsource "$path/cube_5med/cube_5med.source"

runcase 10000000 "cube_5med"

loadmesh "$path/fourlayer/FourLayer.mesh"
loadmaterials "$path/fourlayer/FourLayer.opt"
loadsource "$path/fourlayer/FourLayer.source"

runcase 10000000 "FourLayer"

#puts "Elapsed:   [expr ($tWritten-$tStart)*1e-3]"
#puts "  Loading: [expr ($tLoaded-$tStart)*1e-3]"
#puts "  Running: [expr ($tFinish-$tLoaded)*1e-3]"
#puts "  Writing: [expr ($tWritten-$tFinish)*1e-3]"
