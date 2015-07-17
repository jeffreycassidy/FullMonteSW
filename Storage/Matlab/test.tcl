load libFullMonteGeometry_TCL.so
load libFullMonteMatlabWriter_TCL.so
load libFullMonteTIMOS_TCL.so

TIMOSReader R "/Users/jcassidy/src/FullMonteSW/FullMonte/mouse"

set M [R mesh]

set extsurf [$M getRegionBoundaryTris 0]

puts "$extsurf"

MatlabWriter W

W setMesh $M
W setComment "This is me testing my new MatlabWriter"
W setFaceSubset $extsurf
W writeFaces "faces.txt"
