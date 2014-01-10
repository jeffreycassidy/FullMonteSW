ssh betzgrp-pcjeff.eecg.utoronto.ca "cd src/PDT/trunk/c++; compareflight --ref $1 --cmp $2; cd ../octave; make comparedoc.pdf;"
scp betzgrp-pcjeff.eecg.utoronto.ca:/home/jcassidy/src/PDT/trunk/octave/comparedoc.pdf ./comparesurf_$1_$2.pdf
ssh betzgrp-pcjeff.eecg.utoronto.ca "cd src/PDT/trunk/c++; compareflight --ref $1 --cmp $2 --dtype 2; cd ../octave; make comparedoc.pdf;"
scp betzgrp-pcjeff.eecg.utoronto.ca:/home/jcassidy/src/PDT/trunk/octave/comparedoc.pdf ./comparevol_$1_$2.pdf
