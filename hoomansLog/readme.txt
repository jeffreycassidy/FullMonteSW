READ ME:

To make FullMonte working on your machine MakeFile.in  in FullMonte directory has to be modified.

1. line Makefile.in line 28, has to be modified to correspond to the directory FullMonte is inside in your computer.

ifndef SOURCE_ROOT 
	SOURCE_ROOT=/home/houman 
endif 

(Next step is to replace it with something generic )

2.The packages used in FullMonte have to be installed separately, these packages are:  

	a. tcl8.5-dev or higher. After downloading these packages the header files needed should automatically go to /usr/include/tcl8.5 and .a files to /usr/lib/x86_64-linux-gnu/ (sudo apt-get install tcl8.5-dev in terminal),
if they are somwhere else, these lines in Makefile.in should be updated correspondingly:

ifndef TCL_LIB 
   	TCL_LIB= /usr/lib/x86_64-linux-gnu 
endif 


   ifndef TCL_INCLUDE 
	TCL_INCLUDE=/usr/include/tcl8.5 
 endif


	b.libboost1.54-all-dev . (sudo apt-get install libboost1.54-all-dev ). check if the header files and library files are in these exact directories. 

ifndef BOOST_INCLUDE 	 
	BOOST_INCLUDE=/usr/local/include 
endif 

ifndef BOOST_LIB 
	BOOST_LIB=/usr/lib/x86_64-linux-gnu 
endif

	
	c.swig3.0 (sudo apt-get install swig3.0)


	d.antlr. Download this file: http://www.antlr3.org/download/C/libantlr3c-3.4.tar.gz and exctract it.
Go to the directory libantlr3c-3.4 and type ./configure; make; make install; . For more info read  libantlr3c-3.4/INSTALL.  Antlr header files and library files should be here:

ifndef ANTLR_INCLUDE
	ANTLR_INCLUDE=/usr/local/include
endif

ifndef ANTLR_LIB
 tcl8.5-dev
	ANTLR_LIB=/usr/local/lib/
endif


**can be improved
	Finally download http://www.antlr3.org/download/antlr-3.5.2-complete-no-st3.jar and in FullMonte/MakeFile.in replace the path with where it is in your computer. 

ifndef ANTLR
	ANTLR=java -jar /home/houman/antlr3/antlr-3.5.2-complete-no-st3.jar

endif



	e.VTK. 
tk8.5-dev is a prerequisite (sudo apt-get install tk8.5-dev in terminal),
Download http://www.vtk.org/files/release/6.1/VTK-6.1.0.tar.gz and extract it.  Open VTK-6.1.0/README.html for detailed instuctions of instalation. 
Build the code (as instructed in VTK-6.1.0/README.html under Building with CMake) and in the ccmake curses dialog Enable VTK_WRAP_TCL option .
Install (as instructed in VTK-6.1.0/README.html under Instalation)
Now, the header files and the library files should be exactly here:

ifndef VTK_INCLUDE

	VTK_INCLUDE =/usr/local/include/vtk-6.1

endif

ifndef VTK_LIB
	VTK_LIB=/usr/local/lib
endif


