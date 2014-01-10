GCC_OPTS=-Wall -O0 -g

test: plottimos
	plottimos ../../data/cube_5med.mesh ../../data/cube_5med.result
	../vis/SurfaceRender output.vtk

all: surfcompare surfcompare2 plottimos

surfcompare: surfcompare.cpp ../newgeom.cpp
	g++ $(GCC_OPTS) $^ -o $@

surfcompare2: surfcompare2.cpp ../newgeom.cpp ../io_timos.cpp ../source.cpp ../helpers.cpp ../graph.cpp ../face.cpp writeFileVTK.cpp
	g++ $(GCC_OPTS) $^ -o $@

plottimos: plottimos.cpp ../newgeom.cpp ../io_timos.cpp ../graph.cpp ../face.cpp ../source.cpp ../helpers.cpp writeFileVTK.cpp
	g++ $(GCC_OPTS) $^ -o $@

clean:
	rm -f surfcompare surfcompare2 plottimos *.o
