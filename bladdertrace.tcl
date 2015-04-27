load libFullMonteTracer.so
load libFullMonteTIMOS_TCL.so

#default file prefix
set pfx "/Users/jcassidy/src/FullMonteSW/humandata/bladder"

#override with 1st cmdline arg
if { $argc >= 1 } { set pfx [lindex $argv 0] }

set meshfn "$pfx.mesh"
set legendfn "$pfx.legend"
set optfn "$pfx.opt"


# create and set up the reader

TIMOSReader R

R setMeshFileName $meshfn
R setOpticalFileName $optfn
R setLegendFileName $legendfn



# Load materials
set mats [R materials_simple]
puts "Read [llength $mats] materials"


# Load legend
set legend [R legend]
puts "Loaded [llength $legend] regions"


# Load mesh and create VTK representation
set mesh [R mesh]

set E 50.0
set units_per_cm 1.0

# point source at 0 3 149

puts $mats

TracerKernel $mesh $mats [newIPS 0 3 149] 1000 $units_per_cm $E
