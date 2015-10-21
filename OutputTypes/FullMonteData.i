%module FullMonteData_TCL

%{
#include <vector>
#include <boost/container/flat_map.hpp>
#include "SparseVector.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "PacketTrace.hpp"
%}

%include "std_string.i"
%include "SparseVector.hpp"
%include "PacketTrace.hpp"
