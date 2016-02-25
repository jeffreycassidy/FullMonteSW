load libFullMonteVTKFileTCL.so

set R [VTKLegacyReader]
VTKLegacyReader_setFileName $R "[lindex $argv 0]"
set mesh [VTKLegacyReader_mesh $R]

set W [VTKLegacyWriter]
VTKLegacyWriter_writeMeshRegions $W "[lindex $argv 1]" $mesh
