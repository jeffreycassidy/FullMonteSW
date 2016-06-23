load libFullMonteSTL_TCL.so

set ifn [lindex $argv 0]
set ofn [lindex $argv 1]

puts "Converting STL binary file '$ifn' to text file '$ofn'"

set p [loadSTLBinary $ifn]

writeSTLText $ofn [pairPT_first_get $p] [pairPT_second_get $p] ""
