load libFullMonteBinFile_TCL.so
load libFullMonteTIMOS_TCL.so

if { $argc >= 1 } { set ipfx [lindex $argv 0] } else { set ipfx "../../../data/mouse" }
if { $argc >= 2 } { set opfx [lindex $argv 1] } else { set opfx "mouse" }


puts "Reading mesh from $ipfx.mesh"
TIMOSReader R $ipfx
set mesh [R mesh]

puts "Writing binary mesh to $opfx.mesh.bin"
BinFileWriter W $opfx
W write $mesh

puts "Reading back from $opfx.mesh.bin"
BinFileReader BR $opfx
set imesh [BR mesh]


