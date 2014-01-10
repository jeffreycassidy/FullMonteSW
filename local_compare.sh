DIR=`pwd`
compareflight --ref $1 --cmp $2 --dtype 1
cd ../octave
make
mv comparedoc.pdf $DIR/comparesurf$1_$2.pdf
cd $DIR
compareflight --ref $1 --cmp $2 --dtype 2
cd ../octave
make
mv comparedoc.pdf $DIR/comparevol$1_$2.pdf
cd $DIR
