vtkXMLUnstructuredGridReader reader
reader SetFileName "geompack_out.xml"

# create a mapper/actor to display the plain surface

#vtkPolyDataNormals normals
#normals SetInputConnection [geomfilter GetOutputPort]

#vtkVolumeRayCastCompositeFunction compositeFunction

vtkDataSetMapper meshmapper
meshmapper SetInputConnection [reader GetOutputPort]
meshmapper ScalarVisibilityOn

vtkActor meshactor
meshactor SetMapper meshmapper
[meshactor GetProperty] SetRepresentationToWireframe


################################################################################
## Clipper

set nx 1.0
set ny 0.0
set nz 0.0

set ox 0.0
set oy 0.0
set oz 0.0


vtkPlane clipplane


vtkClipDataSet clipper
clipper SetInputConnection [reader GetOutputPort]
clipper SetClipFunction clipplane

#setClip 0.0 0.0 0.0 0.0 0.0

vtkDataSetMapper clipmapper
clipmapper SetInputConnection [clipper GetOutputPort]

vtkActor clipactor
clipactor SetMapper clipmapper
[clipactor GetProperty] SetRepresentationToWireframe


################################################################################
## Surface extractor

vtkGeometryFilter surfacefilter
surfacefilter SetInputConnection [reader GetOutputPort]

vtkWindowedSincPolyDataFilter smoother
smoother SetInputConnection [surfacefilter GetOutputPort]
smoother SetNumberOfIterations 50

vtkPolyDataNormals normals
normals SetInputConnection [smoother GetOutputPort]

vtkReverseSense reverse
reverse SetInputConnection [normals GetOutputPort]
reverse ReverseCellsOn
reverse ReverseNormalsOn
reverse Update

vtkPolyDataMapper surfacemapper
surfacemapper SetInputConnection [reverse GetOutputPort]
surfacemapper ScalarVisibilityOff

vtkActor surfaceactor
surfaceactor SetMapper surfacemapper
[surfaceactor GetProperty] SetRepresentationToSurface
#[surfaceactor GetProperty] SetOpacity 0.5
[surfaceactor GetProperty] SetColor 1 .75 .75 


################################################################################
## Volume mapper

#vtkProjectedTetrahedraMapper volmapper
#volmapper SetInputConnection [reader GetOutputPort]
##volmapper SetVolumeRayCastFunction compositeFunction
#
#vtkVolume vol
#vol SetMapper volmapper


#################################################################################
### Surface actor
#
#vtkDataSetMapper surfacemapper
#surfacemapper SetInputConnection [reader GetOutputPort]
#surfacemapper ScalarVisibilityOn
#
#vtkActor surfaceactor
#surfaceactor SetMapper surfacemapper
#[surfaceactor GetProperty] SetRepresentationToSurface

#vtkActor volactor
#volactor SetMapper volmapper
#[volactor GetProperty] SetRepresentationToSurface

################################################################################
## Renderer

vtkRenderer ren
#ren AddViewProp vol
#ren AddActor surfaceactor
#ren AddActor meshactor
ren AddActor clipactor
vtkRenderWindow renwin
renwin AddRenderer ren

vtkRenderWindowInteractor iren
iren SetRenderWindow renwin
renwin Render
#iren Start

#wm withdraw .


proc setClip { ox oy oz phi lambda args } {

    set lambda_rad [expr $lambda/180.0*3.141592654];
    set phi_rad   [expr $phi/  180.0*3.141592654];

    set sinlambda [expr sin($lambda_rad)]
    set coslambda [expr cos($lambda_rad)]
    set sinphi    [expr sin($phi_rad)]
    set cosphi    [expr cos($phi_rad)]

    clipplane SetNormal [expr $cosphi*$sinlambda] [expr $cosphi*$coslambda] $sinphi
    clipplane SetOrigin $ox $oy $oz
#    clipper Update

    renwin Render
}

scale .origin_x -label {Origin X} -from -20 -to 20 -variable ox -command { setClip $ox $oy $oz $phi $lambda }
scale .origin_y -label {Origin Y} -from -20 -to 20 -variable oy -command { setClip $ox $oy $oz $phi $lambda }
scale .origin_z -label {Origin Z} -from -20 -to 20 -variable oz -command { setClip $ox $oy $oz $phi $lambda }
scale .normal_phi -label {Rotation phi} -from -180 -to 180 -variable phi -command { setClip $ox $oy $oz $phi $lambda }
scale .normal_lambda -label {Rotation lambda} -from -180 -to 180 -variable lambda -command { setClip $ox $oy $oz $phi $lambda }

pack .origin_x
pack .origin_y
pack .origin_z
pack .normal_phi
pack .normal_lambda

