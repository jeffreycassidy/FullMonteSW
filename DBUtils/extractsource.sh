#!/bin/bash

echo "select * from sources join sources_vol on sources_vol.sourceid=sources.sourceid where sourcetype='V' and sourcegroupid=42;" | psql --host $FULLMONTE_DBHOST --user $FULLMONTE_DBUSER FMDB
