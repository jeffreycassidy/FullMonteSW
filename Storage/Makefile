include ../Makefile.in

SUBDIRS=CommonParser NIRFAST Postgres STL TIMOS

clean:
	for d in $(SUBDIRS); do make -C $$d clean; done
	
libs:
	for d in $(SUBDIRS); do make -C $$d libs; done
