#include "sse.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include "graph.hpp"

#include <algorithm>

#include "fluencemap.hpp"
//
//
//MeshMapper listSurface(const TetraMesh* M,unsigned region)
//{
//    unsigned i=1,j=0;
//
//    vector<unsigned long> Pmap(M->getNp()+1,0);
//    vector<unsigned long> Fmap(M->getNf()+1,0);
//    vector<unsigned long> Tmap;
//
//    for(auto it = M->vecFaceID_Tetra.begin()+1; it != M->vecFaceID_Tetra.end(); ++it,++i)
//    {
//    	// check if the face touches the outside world
//        if (M->T_m[it->first] != M->T_m[it->second] && (M->T_m[it->first]==region || M->T_m[it->second] == region))
//        {
//        	// mark face
//            Fmap[i] = ++j;
//
//            // mark points on face
//            for(const unsigned& p : M->getFacePointIDs(i))
//            	Pmap[p]=1;
//        }
//    }
//    unsigned Nps = count(Pmap.begin()+1,Pmap.end(),1);
//    cout << "Region code " << region << ": Found " << j << " surface faces, " << Nps << " surface points (of a total " << Pmap.size() << ")" << endl;
//
//    j=0;
//    for(unsigned long& pm : Pmap)
//    	pm = (pm == 1) ? ++j : 0;
//
//    MeshMapper mm(*M,Pmap,Fmap,Tmap);
//
//    cout << "  Output " << j << " points" << endl;
//
//    return mm;
//}
