rsync -rizt Makefile *.?pp betzgrp-pcjeff.eecg.utoronto.ca:/home/jcassidy/src/PDT/trunk/c++
ssh betzgrp-pcjeff.eecg.utoronto.ca "bash -c \"cd src/PDT/trunk/c++; make && ./montecarlo --input ../data/cube_5med.mesh --sourcefile ../data/cube_5med.source --materials ../data/cube_5med.opt --N 100000\""
