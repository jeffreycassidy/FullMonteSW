# should be compiled with -DLOG_TEXT
montecarlo --input ../octave/lamina_r10_n2.geom.txt --materials ../data/twolayer.opt --N 1000
mv log.out log_refract.out
../haskell/Output log_refract.out 200
montecarlo --input ../octave/lamina_r10_n2.geom.txt --materials ../data/twolayer_rev.opt --N 1000
mv log.out log_refract_rev.out
../haskell/Output log_refract_rev.out 200
