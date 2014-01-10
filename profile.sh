rm gmon.out
montecarlo -c 1 --N 1000000 --nodbwrite
cp gmon.out gmon.c1_1M.out
gprof montecarlo gmon.c1_1M.out > profile.c1_1M.txt

rm gmon.out
montecarlo -c 6 --N 1000000 --nodbwrite
cp gmon.out gmon.c6_1M.out
gprof montecarlo gmon.c6_1M.out > profile.c6_1M.txt
