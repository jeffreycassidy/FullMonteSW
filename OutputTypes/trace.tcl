package require vtk
load libFullMonteData_TCL.so
load libFullMonteGeometry_TCL.so
load libFullMonteTIMOS_TCL.so
load libFullMonteVTK.so

#set pt [PacketTrace_loadTextFile "/Users/jcassidy/src/FMHW/source/bluespec/output.trace.txt"]

set traceFn "/Users/jcassidy/src/FMHW/source/bluespec/output.trace.txt"
set oTraceFn "traces.vtk"

set meshFn "/Users/jcassidy/src/FullMonteSW/data/cube_5med.mesh"


#### Do the trace conversion
convertTraces $traceFn $oTraceFn


TIMOSReader R
R setMeshFileName $meshFn
set M [R mesh]

puts "Tetra enclosing (0,0,0) is [TetraMesh_findEnclosingTetra $M {0 0 0}]"

VTKMeshRep VM $M


for { set i 0 } { $i <= 5 } { incr i } {
    set surfpd($i) [VM getSurfaceOfRegion $i]
    vtkPolyDataWriter W
    W SetInputData $surfpd($i)
    W SetFileName "surface.$i.vtk"
    W Update
    W Delete
}

