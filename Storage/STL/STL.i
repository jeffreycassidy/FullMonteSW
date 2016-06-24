%module FullMonteSTL_TCL

%include "std_pair.i"
%include "std_string.i"

%template(pairPT) std::pair<std::vector<std::array<float,3> >,std::vector<std::array<unsigned,3> > >;

%{

#include <vector>
#include <iostream>
#include "STLBinary.hpp"
#include "STLText.hpp"

%}

%include "STLBinary.hpp"
%include "STLText.hpp"