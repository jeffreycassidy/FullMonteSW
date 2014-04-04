#include "sse.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "graph.hpp"

#include "MeshMapper.hpp"

int main(int argc,char **argv)
{
    boost::shared_ptr<PGConnection> dbconn=PGConnect();

    TetraMesh *M = exportMesh(*(dbconn.get()),1);

    listSurface(M);
}

void listSurface(TetraMesh* M)
{
    unsigned i=0,j=0;
    unsigned boundary;
    vector<unsigned long> Pmap(M->getNp()+1,0);
    vector<unsigned long> Fmap(M->getNf()+1,0);

    for(auto it = M->vecFaceID_Tetra.begin()+1; it != M->vecFaceID_Tetra.end(); ++it,++i)
    {
        if (it->first==0 || it->second == 0)
            Fmap[i] = ++j;
    }
    cout << "Found " << j << " surface faces" << endl;

    for(auto it : vector_index_adaptor(Fmap,0))
        cout << it.first << ": " << it.second << endl;
}
