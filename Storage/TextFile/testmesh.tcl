load libFullMonteTextFileTCL.so
load libFullMonteTIMOSTCL.so
load libFullMonteGeometryTCL.so

TIMOSAntlrParser R
R setMeshFileName /Users/jcassidy/src/FullMonteSW/data/TIM-OS/mouse/mouse.mesh
set M [R mesh]

TriFilterRegionBounds surf
surf mesh $M
surf includeRegion 0 1

TextFileMeshWriter W
W fileName "out.mesh"
W faceFilter surf
W mesh $M
W write
