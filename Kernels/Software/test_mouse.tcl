source test_config.tcl

TIMOSAntlrParser R
	R setMeshFileName "$datapath/TIM-OS/mouse/mouse.mesh"
	R setSourceFileName "$datapath/TIM-OS/mouse/mouse.source"
	R setOpticalFileName "$datapath/TIM-OS/mouse/mouse.opt"
	
set mesh [R mesh]
set source [R sources]
set materials [R materials_simple]

TetraSVKernel SV $mesh

SV materials	$materials
SV packetCount 	1000000
SV rouletteWMin	1e-5
SV threadCount	8

set b [Ball_New]
$b centre "10 40 11"
$b radius 2.0

SV source $b

SV startAsync

while { ![SV done] } {
	puts -nonewline "\rProgress: [format %5.2f%% [expr 100.0*[SV progressFraction]]]"
	flush stdout
	after 1000
}

$b radius 40
SV source $b


SV startAsync

while { ![SV done] } {
	puts -nonewline "\rProgress: [format %5.2f%% [expr 100.0*[SV progressFraction]]]"
	flush stdout
	after 1000
}

SV finishAsync