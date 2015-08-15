Installation
============

Prerequisites

Modern compiler with C++11 support (G++ >= 4.8 tested and works)

Boost 1.56.0 or higher (<1.56.0 will break) - www.boost.org
VTK 6.1.0 with Tcl bindings - www.vtk.org

Antlr3 java package v3.5.2 - www.antlr3.org/download.html
Antlr3 C runtime v3.4 - www.antlr3.org/download/C
** When building Antlr3c, be sure to select --enable-64bit when using configure; it defaults to 32bit **

Swig 3.0 - www.swig.org (also often a package in Linux)


Path setup

Paths for several of the libraries and tools must be specified in Makefile.in


On Ubuntu 14.04 LTS the following build procedure works:

G++ 4.8 | sudo apt-get install g++-4.8

Boost 1.56.0 | Download and install Boost 1.56.0 or higher using their instructions

Tcl/Tk/OpenGL | sudo apt-get install freeglut3-dev tcl8.5-dev tk8.5-dev swig3.0
VTK 6.1.0 | Build & install VTK 6.1.0 using "ccmake ." to configure

Antlr3 JAR | Copy to a known location

Antlr3c runtime | Unpack Antlr3c runtime
                | ./configure --enable-64bit --prefix=/usr/local/antlrv3/antlr3c-3.4
                | make
                | sudo make install

Configure paths to Boost, Antlr, SWIG executable, etc in Makefile.in

Build FullMonte


Building
========

Once the prerequisites are installed, simply type 'make libs' which will build the FullMonte libraries



Testing
=======

TBC
