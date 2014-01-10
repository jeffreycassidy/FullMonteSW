exportResult -R $1 --dtype 1; mv op.txt output.r$1.surf.txt
exportResult -R $2 --dtype 1; mv op.txt output.r$2.surf.txt

LOGFILE=compare-digi-1G.txt

echo "Comparing runs $1 and $2" > $LOGFILE

echo "SURFACE" >> $LOGFILE
octave -q --eval "addpath('../octave'); fn1='output.r$1.surf.txt'; fn2='output.r$2.surf.txt'; doCompare;" # |tee -a $LOGFILE

mv quadpanel.eps ~/src/thesis/surface-digi-1G-$1-$2.eps
mv quadpanel.png ~/src/thesis/surface-digi-1G-$1-$2.png

exportResult -R $1 --dtype 2; mv op.txt output.r$1.vol.txt
exportResult -R $2 --dtype 2; mv op.txt output.r$2.vol.txt

echo "VOLUME" >> $LOGFILE
octave -q --eval "addpath('../octave'); fn1='output.r$1.vol.txt'; fn2='output.r$2.vol.txt'; doCompare;" #|tee -a $LOGFILE

mv quadpanel.eps ~/src/thesis/vol-digi-1G-$1-$2.eps
mv quadpanel.png ~/src/thesis/vol-digi-1G-$1-$2.png
