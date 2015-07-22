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

W setFacesToAll
DoubleVector phi 306800 0
phi set 0 5
phi set 1 5
phi set 2 2
phi set 3 5
phi set 4 5
phi set 5 5
phi set 6 5
phi set 7 5
phi set 8 5
phi set 9 5

W setComment "No threshold, all faces"
W writeSurfaceFluence "test.noth.out" phi

W threshold 2
W setComment "Threshold 2, expect results at 0=5, 2=5, 5=2"
W writeSurfaceFluence "test.th2.out" phi

W removeThreshold
W setComment "Threshold removed"

