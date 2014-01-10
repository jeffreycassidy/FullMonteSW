
echo "SELECT casename,cmp_runid,ref_runid,ref_usertime,cmp_usertime,usertime_pct FROM perfcompare($1,$2);" | psql -U postgres -d FMDB
