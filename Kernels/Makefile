include ../Makefile.in

SUBDIRS= Software

libs: sublibs libFullMonteKernel.so libFullMonteKernel_TCL.so
	
sublibs: 
	for d in $(SUBDIRS); do make -C $$d libs; done;
	
clean:
	for d in $(SUBDIRS); do make -C $$d clean; done;
	
libFullMonteKernel.so: FullMonteKernel.o OStreamObserver.o
	$(CXX) $(CXX_LIB_OPTS) -L$(BOOST_LIB) -lboost_system -lFullMonteGeometry -lFullMonteCore -L$(SOURCE_ROOT)/FullMonte -lSFMT -o $@ $^
	
libFullMonteKernel_TCL.so: Kernel_wrap.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $< -lFullMonteKernel -lFullMonteGeometry -lSFMT -lFullMonteData -lFullMonteCore -lboost_system -ltclstub8.5

OStreamObserver.o: Software/OStreamObserver.cpp
	$(CXX) $(CXX_OPTS) -c $<
