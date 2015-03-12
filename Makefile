# base options only

default: libs

include Makefile.in

Geometry/%:
	make -C Geometry $*
	
Storage/TIMOS/%:
	make -C Storage/TIMOS $*
	
VTK/%:
	make -C VTK $*
	
SFMT/%:
	make -C SFMT $*

libs: Geometry/libFullMonteGeometry.so Storage/TIMOS/libFullMonteTIMOS.so Storage/TIMOS/libFullMonteTIMOS_TCL.so SFMT/libSFMT.a \
	VTK/libFullMonteVTK.so libFullMonteCore.so libFullMonteBLI.so libFullMonteBLI_TCL.so

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $< -o $@
	


##### Core functions (RNG, basic loggers, result types, and OStreamObserver)

random.o: random.cpp random.hpp
	$(CXX) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

libFullMonteCore.so: LoggerSurface.o LoggerConservation.o LoggerEvent.o sse.o  RandomAVX.o OStreamObserver.o FullMonte.o \
	fluencemap.o LoggerEvent.o LoggerVolume.o FullMonte.o
	$(CXX) $(CXX_OPTS) -shared -L. -lFullMonteGeometry -LSFMT -lSFMT -LGeometry -lboost_system -lboost_timer -lboost_chrono -o $@ $^


##### BLI-kernel related items

libFullMonteBLI.so: BLIKernel.o
	$(CXX) $(CXX_OPTS) -shared -L. -lFullMonteGeometry -LGeometry -lboost_system -lboost_chrono -lFullMonteCore -o $@ $^



##### TCL bindings for BLI applications

BLIKernel_wrap.cxx: BLIKernel.i
	swig -tcl -c++ $<
	
BLIKernel_wrap.o: BLIKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -DUSE_TCL_STUBS -c $<

libFullMonteBLI_TCL.so: BLIKernel_wrap.o
	$(CXX) -shared $(CXX_OPTS) -ltclstub8.5 -L. -lFullMonteBLI -LGeometry -lFullMonteGeometry -o $@ $^


#### Cleanup targets

veryclean: clean
	make -C Geometry clean
	make -C Storage/TIMOS clean
	make -C Storage/CommonParser clean
	make -C VTK clean
	make -C SFMT clean

clean:
	rm -rf *.o *.a *.so *.dSYM *_wrap.cxx
