%module FullMonteDataTCL

%{
#include <vector>
#include <boost/container/flat_map.hpp>
#include "SparseVector.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include "PacketPositionTrace.hpp"
%}

%include "std_string.i"
%include "SparseVector.hpp"
%include "PacketPositionTrace.hpp"

%template(SparseVectorUF) SparseVector<unsigned,float>;