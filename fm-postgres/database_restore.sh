if [ -z "$1" ]; then
    file="FMDB_backup.database";
else
    file=$1;
fi

sudo -u postgres pg_restore -c -d FMDB $file
