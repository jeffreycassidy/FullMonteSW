# About

FullMonte
(c) 2012-2016 Jeffrey Cassidy and the University of Toronto

Contains SFMT (Matsumoto and Saito's SIMD-Oriented Fast Mersenne Twister) in SFMT/, which is subject to separate copyright and
license provisions (see `SFMT/LICENSE`). The `CMakeLists.txt` file contained there is my own work, but the files are otherwise the original SFMT-1.4.1 from the <a href="www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT">authors' website</a>.

The files in `data/TIM-OS` contain the TIM-OS test suite <a href="https://sites.google.com/a/imaging.sbes.vt.edu/tim-os">available here</a> from Shen and Wang (2010), including a version of Dogdas et al's <a href="http://neuroimage.usc.edu/neuro/Digimouse">Digimouse</a> (2007).

Contains <a href="gruntthepeon.free.fr/ssemath">Julien Pommier</a>'s "ssemath.h" SSE math library which is available under the zlib license as `Kernels/Software/sse_mathfun.h`. The SSE implementations were extended to larger vector lengths using AVX instructions `avx\_mathfun.h`.


## Citations

If using in academic publications, please cite:

XXXXX

Use of data from Digimouse or TIM-OS should also cite appropriately as specified on the websites listed above.


## License

TBD





# Installation

FullMonte uses the CMake (www.cmake.org) packaging system to support multiple OSes, compilers, IDEs, and build systems. It should
compile relatively easily on Unix-like operating systems with modern processors (AVX instruction set minimum).


## Prerequisites

### Required

CMake version >= 3.1

Modern compiler with C++11 support (G++ >= 4.8, Clang tested and work)

Boost version >= 1.56.0 with serialization, iostreams, system, program\_options unit\_test\_framework compiled - www.boost.org


ANTLR3 (parser generator): - <a href="www.antlr3.org/download.html">java package v3.5.2 </a> and <a href="www.antlr3.org/download/C">C runtime 3.4</a> 
**When building Antlr3c, be sure to select --enable-64bit when using configure; it defaults to 32bit**


### Optional 

VTK 6.3.0 with Tcl bindings (set 'VTK_TCL_WRAP=ON') (for visualization and export of VTK/Paraview files) www.vtk.org

Paraview (for result visualization; requires VTK) - www.paraview.org

Tcl/Tk/OpenGL (for Tcl script bindings) on Linux try sudo apt-get install freeglut3-dev tcl8.5-dev tk8.5-dev

Swig 3.0 (for generating Tcl script bindings) - www.swig.org (also often a package in Linux, eg "swig" or "swig3.0")


#### Qt support (GUI in progress)

Download Qt 4.4 from <a href="http://www.qt.io/download-open-source">qt.io</a> and build following instructions


## Configuration

Out-of-source build is recommended using CMake. A typical developer's use case would have separate Debug and Release builds.
Unpack the source in a folder, and create a separate folder elsewhere for the build directory.

```
/path/to/build >  ccmake /path/to/source
```

You may find it necessary to specify paths to packages such as Boost (`ccmake -DBOOST_DIR=/path/to/boost`). CMake documentation
will help with standard packages (VTK, Tcl/Tk, Swig) and compiler settings. ANTLR must be configured separately with `ANTLR3_INCLUDE_DIR` pointing to the folder holding `antlr3.h`, `ANTLR3_LIB_DIR` pointing to the folder holding `libantlr3c.a`, and `ANTLR_JAR` providing the full path to the ANTLR compiler .jar file. 

To output VTK files or perform visualization, the `WRAP_VTK` option must be enabled during build.

**Ensure that build type is set to release for full performance. Architecture should be set appropriately to "avx" or "avx2".**


## 

### Using G++ on Mac OS

Be aware that G++ and Clang binaries aren't necessarily compatible due to name-mangling, eg. if you build Boost or VTK with Clang
then you'll need to use Clang to build FullMonte as well.

The compiler can be specified when CMake is _first_ run by using the CMAKE_C_COMPILER and CMAKE_CXX_COMPILER options.
To change the compiler, you must nuke the build directory and re-run CMake (this is a CMake quirk).

The script below can be used as an example (**Make sure to specify an absolute path whenever a path is needed**).
**Also, make sure you have an 'include' directory in "/usr/lib/jvn/<any java installation>** If there is not any, re-install the same java version, and then export JAVA_HOME to be the path to that java installation.  

```
mkdir -p FullMonteSW/Build/ReleaseGCC
cd FullMonteSW/Build/ReleaseGCC
ccmake \
    -DCMAKE_MODULE_PATH=/Users/jcassidy/src/FMClean/FullMonteSW/cmake\
    -DARCH=AVX2 \
    -DCMAKE_CXX_COMPILER=/sw/bin/g++-fsf-4.9 \
    -DCMAKE_C_COMPILER=/sw/bin/gcc-fsf-4.9 \
    -DBOOST_ROOT=/sw/opt/boost-1_58 \
    -DWRAP_TCL=ON \
    -DWRAP_VTK=ON \
    -DCMAKE_BUILD_TYPE=Release\
    '-DCMAKE_CXX_FLAGS=-Wall -Wa,-q'\
    '-DCMAKE_C_FLAGS=-Wall -Wa,-q'\
    ../..
make
```

Some packages that the user should make sure to have are:
1- libproj-dev
2- rlwrap

### Broken Tcl wrapping in VTK 6.1.0

VTK's Tcl wrapping export is broken in version 6.1.0 (and possibly others). To fix:


/usr/local/lib/cmake/vtk-6.1/vtkWrapTcl.cmake

Remove the COPYONLY argument at line 179 (delete or comment out by prepending #).


/usr/local/lib/cmake/vtk-6.1/vtkWrapperInit.data.in

Ensure the variable name enclosed in @@ is all uppercase.



# Testing

## Basic simulation kernel

Basic functionality can be checked by running `Kernels/Software/Test_Mouse` which is a basic bioluminescence simulation based on the
Digimouse test data released with TIM-OS by Shen and Wang (2010). It loads the mesh, optical properties, and source definitions, checks
that the data is loaded correctly (number of mesh elements, sources, etc), then runs the simulator and checks that the basic energy-conservation laws are followed, and that the event counts are approximately correct.

If FullMonte was built with `WRAP_VTK` enabled, then it will also output .vtk files showing the surface and volume fluence, which can
be loaded into Paraview (using the `VisualizeMouse.pvsm` state file).


## Unit Tests

Many folders contain a `Test/` subfolder with Boost Unit Test Framework unit tests for specific classes.
