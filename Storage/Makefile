include ../Makefile.in

SUBDIRS=CommonParser Postgres STL TIMOS TetGen BinFile Matlab VTK VTKLegacy

clean:
	for d in $(SUBDIRS); do make -C $$d clean; done
	
libs:
	for d in $(SUBDIRS); do make -C $$d libs; done
