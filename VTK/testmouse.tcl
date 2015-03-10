package require vtk

load libFullMonteVTK.so
load libFullMonteTIMOSTCL.so

TIMOSReader R "/Users/jcassidy/src/FullMonteSW/data/mouse"

# Load legend
set legend [R legend]
puts "Loaded [llength $legend] regions"

# Parse the mesh
set mesh [R mesh]

# Create VTK mesh representation
VTKMeshRep V $mesh

set ug [V getMeshWithRegions]

puts "Extracted cell mesh, total [$ug GetNumberOfCells] cells and [$ug GetNumberOfPoints] points"


# set up VTK render window and interactor

vtkRenderer ren

vtkRenderWindow renwin
    renwin AddRenderer ren

vtkRenderWindowInteractor iren
    iren SetRenderWindow renwin
    [iren GetInteractorStyle] SetCurrentStyleToTrackballCamera





# actor: legend

for { set i 0 } { $i < [llength $legend] } { incr i } {
    V addLegendEntry [lindex $legend $i]
    puts "[LegendEntry_label_get [lindex $legend $i]]"
}

set legendactor [V getLegendActor "0.5 0.1" "0.9 0.9"]




# actor: surface dataset

vtkDataSetMapper dsm
    dsm SetInputData $ug
    dsm SetLookupTable [V getRegionMapLUT]

vtkActor actor
    actor SetMapper dsm
    [actor GetProperty] SetColor 1.0 1.0 1.0
ren AddActor actor


ren AddViewProp $legendactor

renwin Render
