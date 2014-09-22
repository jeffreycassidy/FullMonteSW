#!/bin/bash

# extracts a volume source from the database, and prints out the list of tetra IDs used in it

echo "select default_w, tetraid from sources join sources_vol on sources_vol.sourceid=sources.sourceid where sourcetype='V' and sourcegroupid=42;" | psql --host $FULLMONTE_DBHOST --user $FULLMONTE_DBUSER FMDB | awk -F '|' '{print $2; }' | sed -ne '3,$p' | sed -e '/rows/d'
