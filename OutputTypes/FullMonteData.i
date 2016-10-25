%module FullMonteDataTCL

%{
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "clonable.hpp"
#include "visitable.hpp"
#include "OutputData.hpp"
#include "SpatialMap.hpp"
#include "OutputDataSummarize.hpp"
%}

%include "std_string.i"

%include "OutputData.hpp"
%include "AbstractSpatialMap.hpp"
%include "SpatialMap.hpp"
%include "OutputDataSummarize.hpp"


