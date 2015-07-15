include ../Makefile.in

SUBDIRS= Software

libs: sublibs libFullMonteKernels.so libFullMonteKernels_TCL.so
	
sublibs: 
	for d in $(SUBDIRS); do make -C $$d libs; done;
	
clean:
	for d in $(SUBDIRS); do make -C $$d clean; done;
	
libFullMonteKernels.so: Kernel.o OStreamObserver.o
	$(CXX) $(CXX_LIB_OPTS) -L$(BOOST_LIB) -lboost_system -lFullMonteGeometry -lFullMonteCore -L$(SOURCE_ROOT)/FullMonte -lSFMT -o $@ $^
	
libFullMonteKernels_TCL.so: Kernel_wrap.o | libFullMonteKernels.so
	$(CXX) $(CXX_LIB_OPTS) -L$(BOOST_LIB) -lFullMonteKernels -lboost_system -lFullMonteGeometry -o $@ $^ $(TCL_LIB)

	
Kernel_wrap.cxx: Kernel.hpp
	$(SWIG) -tcl -c++  -DSWIG_OPENBRACE=%{ -DSWIG_CLOSEBRACE=%} -o $@ $^
	
Kernel_wrap.o: Kernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $^
	
Kernel.o: Kernel.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $<
	
OStreamObserver.o: Software/OStreamObserver.cpp
	$(CXX) $(CXX_OPTS) -c $<
