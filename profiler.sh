montecarlo-trace -c 1 -N 100000 -t 4 --nodbwrite
mkdir trace-digi-std
mv *.bin trace-digi-std

montecarlo-trace -c 10 -N 100000 -t 4 --nodbwrite
mkdir trace-digi-hi
mv *.bin trace-digi-hi

montecarlo-trace -c 11 -N 100000 -t 4 --nodbwrite
mkdir trace-digi-lo
mv *.bin trace-digi-lo

cd trace-digi-std
../rletrace tetra.trace.0.bin

cd ../trace-digi-hi
../rletrace tetra.trace.0.bin

cd ../trace-digi-lo
../rletrace tetra.trace.0.bin


