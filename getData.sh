if [ -z "$1" ]; then
    echo "Requires one argument (flight ID)"
    exit;
fi

rm -f runlist.txt
psql --host betzgrp-pcjeff.eecg.utoronto.ca --user postgres FMDB -tqc "select * from runs_data where flightid=$1 and datatype in (1,2);" -o runlist.txt

for i in `cut -f 9 -d\| runlist.txt`; do
    echo $i;
    blobexport $i blob.out;
done;
