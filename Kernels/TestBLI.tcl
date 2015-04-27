load libFullMonteKernels_TCL.so
load libFullMonteTIMOS_TCL.so

TetraSurfaceKernel k


TIMOSReader r "/Users/jcassidy/src/FullMonteSW/data/mouse"

set mats [r materials_simple]
set M    [r mesh]
set src  [r sources]

puts "Loaded test case"

puts "mats=$mats"

# Kernel properties
k setSources            $src
k setEnergy             50
k setMaterials          $mats
k setUnitsToMM

# Monte Carlo kernel properties
k setRoulettePrWin      0.1
k setRouletteWMin       1e-5
k setMaxSteps           10000
k setMaxHits            100
k setPacketCount        1000000
k setThreadCount        8
k setRandSeed           1

# Tetra mesh MC kernel properties
k setMesh               $M


# Launch it
k startAsync

puts "Launched"

# display progress
while { ![k done] } {
    after 100
    puts "TCL progress says: [k getProgressFraction]"
}

k awaitFinish

set levents [k getResult "logger.results.events"]
set lsurf   [k getResult "logger.results.surface.energy"]
set lfoobar [k getResult "logger.results.foobar"]

puts "surface: $lsurf"
$lsurf summarize
puts "foobar:  $lfoobar"
puts "events:  $levents"
$levents summarize

# look at results
