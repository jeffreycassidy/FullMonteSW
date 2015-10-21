load libFullMonteData_TCL.so
load libFullMonteGeometry_TCL.so

set sp [SparseVectorUD_loadTextFile "/Users/jcassidy/src/FMHW/source/bluespec/output.absorb.txt"]

puts "Loaded [SparseVectorUD_nnz $sp] nonzero elements"

set Es [SparseVectorUD_dense $sp 48001]

puts "$Es"
