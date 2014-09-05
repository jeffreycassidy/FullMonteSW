#pragma once
#include "SourceDescription.hpp"
#include "graph.hpp"

#include "linefile.hpp"
#include "fluencemap.hpp"

// io_timos.hpp; Contains function prototypes for reading from TIM-OS format files
//  see https://sites.google.com/a/imaging.sbes.vt.edu/tim-os/usage for TIM-OS details

// load a surface with time-varying fluence values; sum across all time steps
template<class T>double readTIMOSOutputFlatten(const TetraMesh& mesh,FluenceMap<T>& F,LineFile& is,unsigned Nd,unsigned Ns);

void readTIMOSOutput(string fn,const TetraMesh&,SurfaceFluenceMap& surf,VolumeFluenceMap& vol);

vector<SourceDescription*> readTIMOSSource(string fn);
vector<Material> readTIMOSMaterials(string fn);
bool writeTIMOSMaterials(string fn,const vector<Material>& mat);

bool writeTIMOSSource(string fn,const vector<SourceDescription*>& s,long long packetcount=0);
