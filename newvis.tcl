package require vtk
load ../TetraMeshTCL.so

# take a surface (without provided normals) and draw it with smoothing and shaded normals
#   surf        Surface object name (vtkPolyData)
#   niter       Number of iterations for windowed sinc smoothing
#
# returns the actor name

proc smoothedsurfactor { surf niter } {
    set geom [vtkGeometryFilter New]
    $geom SetInputData $surf

    $geom Update
        
    #smooth the input
    set smoother [vtkWindowedSincPolyDataFilter New]
    $smoother SetNumberOfIterations $niter
    $smoother SetInputConnection [$geom GetOutputPort]

    # add normals
    set normals [vtkPolyDataNormals New]
    $normals SetInputConnection [$smoother GetOutputPort]

    $normals Update

    set mapper [vtkPolyDataMapper New]
    $mapper SetInputConnection [$normals GetOutputPort]
    $mapper ScalarVisibilityOff

    set actor [vtkActor New]
    $actor SetMapper $mapper

    return $actor
}


# create an actor for a contoured surface
proc contouractor { voldata niter args } {
#    puts "contouractor called with [llength $args] args"
    set c2p [vtkCellDataToPointData New]
    $c2p SetInputData $voldata
    $c2p PassCellDataOn
    $c2p Update

    # create contour filter 
    set contours [vtkContourFilter New]
    $contours SetInputConnection [$c2p GetOutputPort]
    $contours SetNumberOfContours [llength $args]
    for { set i 0 } { $i < [llength $args] } { incr i } { $contours SetValue $i [lindex $args $i] }
#    $contours Update

    set smoother [vtkWindowedSincPolyDataFilter New]
    $smoother SetNumberOfIterations $niter
    $smoother SetInputConnection [$contours GetOutputPort]

    # generate normals for the contours
    set normals [vtkPolyDataNormals New]
    $normals SetInputConnection [$smoother GetOutputPort]

#    $normals Update

    set mapper [vtkPolyDataMapper New]
    $mapper SetInputConnection [$normals GetOutputPort]
    $mapper ScalarVisibilityOff

    set actor [vtkActor New]
    $actor SetMapper $mapper
    [$actor GetProperty] SetOpacity     0.8
    [$actor GetProperty] SetColor       0.8 0.7 0.7 

    return $actor
}



proc regionboundaryactor { voldata regioncodes reg } {
    set tf [vtkThreshold New]
    $tf SetInputConnection [$voldata GetOutputPort]
    $tf SetAttributeModeToUseCellData
    $tf ThresholdBetween $reg $reg

    set surffilter [vtkDataSetSurfaceFilter New]
    $surffilter SetInputConnection [$th GetOutputPort]
    $surffilter UseStripsOff
    $surffilter PassThroughCellIdsOn

    set mapper [vtkPolyDataMapper New]
    $mapper SetInputConnection [$surffilter GetOutputPort]

    set actor [vtkActor New]
    $actor SetMapper $mapper

    return $actor
}

#proc boundaryactor { voldata } { 
#    set surffilter [vtkDataSetSurfaceFilter New]
#    $surffilter SetInputConnection [$voldata GetOutputPort]
#
#    set surfmapper [vtkPolyDataMapper New]
#
#
#    set surfactor [vtkActor New]
#    $surfactor
#}



proc linesourceactor { p0 p1 r } {
    set ls [vtkLineSource New]
    eval "$ls SetPoint1 $p0"
    eval "$ls SetPoint2 $p1"

    set tubes [vtkTubeFilter New]
    $tubes SetRadius $r
    $tubes SetInputConnection [$ls GetOutputPort]
    $tubes SetNumberOfSides 10
    $tubes CappingOn

    set mapper [vtkPolyDataMapper New]
    $mapper SetInputConnection [$tubes GetOutputPort]
    $mapper ScalarVisibilityOff

    set actor [vtkActor New]
    $actor SetMapper $mapper
    [$actor GetProperty] SetOpacity 1.0
    [$actor GetProperty] SetColor 1.0 0.0 0.0
    return $actor
}

set ren [vtkRenderer New]
set renwin [vtkRenderWindow New]
$renwin AddRenderer $ren

# Set up depth peeling (requires NVIDIA card?)
# vtk.org/Wiki/VTK/Examples/Cxx/Visualization/CorrectlyRenderingTranslucentGeometry
$renwin SetAlphaBitPlanes 1
$renwin SetMultiSamples 0
$ren SetUseDepthPeeling 1
$ren SetMaximumNumberOfPeels 8

# set up parameters to load

# run ID
set IDr 3522

# mesh ID (should be implied by runID)
set IDm 1

set conn [tclConnect]

set mesh [loadMesh $conn $IDm]

set phi_v [exportResultVector $conn $IDr 2]

set td [getVTKTetraMesh $mesh]

[$td GetCellData] SetActiveScalars "phi_v"
[$td GetCellData] SetScalars [getVTKDataArray $phi_v]

set ca [contouractor $td 50 10000 1000 100]
set surfa [smoothedsurfactor $td 50]
set lsa [linesourceactor { 11.4 42.4 10.4 } { 11.7 41.6 11 } 0.1]

[$surfa GetProperty] SetOpacity 0.15
[$surfa GetProperty] SetColor 0.5 0.5 0.5

$ren AddActor $surfa
$ren AddActor $ca
$ren AddActor $lsa


# DRAW A BOUNDARY AROUND REGION 7
#set mat 12

#set rpd [createVTKBoundary $mesh $mat]

set a_liver [smoothedsurfactor [createVTKBoundary $mesh 12] 50]
set a_lung  [smoothedsurfactor [createVTKBoundary $mesh 10] 50]
set a_heart [smoothedsurfactor [createVTKBoundary $mesh  3] 50]


set tumourtets [loadVector "../tumour_tet_IDs.txt"]
set pd_tumour [getVTKRegion $mesh $tumourtets]

vtkPolyDataMapper map_tumour
map_tumour SetInputData $pd_tumour

vtkActor tumour
[tumour GetProperty] SetOpacity 0.8
[tumour GetProperty] SetColor 1.0 0.0 0.0
tumour SetMapper map_tumour

$ren AddActor tumour


[$a_liver GetProperty] SetOpacity 0.3
[$a_liver GetProperty] SetColor   0.5 1.0 0.5

[$a_lung  GetProperty] SetOpacity 0.3
[$a_lung  GetProperty] SetColor   1.0 1.0 0.5

[$a_heart GetProperty] SetOpacity 0.3
[$a_heart GetProperty] SetColor   0 0 1

$ren AddActor $a_liver
$ren AddActor $a_lung
$ren AddActor $a_heart

#set mapper [vtkPolyDataMapper New]
#$mapper SetInputData $rpd

#set actor [vtkActor New]
#$actor SetMapper $mapper

#$ren AddActor $actor

##END

set iren [vtkRenderWindowInteractor New]
    $iren SetRenderWindow $renwin
    [$iren GetInteractorStyle] SetCurrentStyleToTrackballCamera
$renwin Render

$iren Start
