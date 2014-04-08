#include "sse.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include "graph.hpp"

#include <algorithm>

#include "Export_VTK_XML.hpp"
#include "MeshMapper.hpp"

#include "fluencemap.hpp"

MeshMapper listSurface(TetraMesh* M);

int main(int argc,char **argv)
{
	// pull mesh from database
    boost::shared_ptr<PGConnection> dbconn=PGConnect();
    TetraMesh *M = exportMesh(*(dbconn.get()),1);

    // do the mapping
    MeshMapper mm = listSurface(M);

    // get surface fluence results
    FluenceMapBase *phi = exportResultSet(dbconn.get(),1234,1,M);
    vector<double> phi_v = phi->toVector(M->getNf());

    // Create output document DOM model
    DOMDocument* doc = xml_createVTKFileDoc();
    DOMElement* el = doc->getDocumentElement();
    xml_createVTKSurfaceMesh(el,M);
	xml_writeFile("output.xml",doc);
	doc->release();


	DOMDocument *doc2 = xml_createVTKFileDoc();
	DOMElement* el2 = doc2->getDocumentElement();
	xml_createVTKSurfaceMesh(el2,&mm,&phi_v);
	xml_writeFile("output_stripped.xml",doc2);
	doc2->release();
}

MeshMapper listSurface(TetraMesh* M)
{
    unsigned i=1,j=0;

    vector<unsigned long> Pmap(M->getNp()+1,0);
    vector<unsigned long> Fmap(M->getNf()+1,0);
    vector<unsigned long> Tmap;

    for(auto it = M->vecFaceID_Tetra.begin()+1; it != M->vecFaceID_Tetra.end(); ++it,++i)
    {
    	// check if the face touches the outside world
        if (it->first==0 || it->second == 0)
        {
        	// mark face
            Fmap[i] = ++j;

            // mark points on face
            for(const unsigned& p : M->getFacePointIDs(i))
            	Pmap[p]=1;
        }
    }
    unsigned Nps = count(Pmap.begin()+1,Pmap.end(),1);
    cout << "Found " << j << " surface faces, " << Nps << " surface points (of a total " << Pmap.size() << ")" << endl;

    j=0;
    for(unsigned long& pm : Pmap)
    	pm = (pm == 1) ? ++j : 0;

    MeshMapper mm(*M,Pmap,Fmap,Tmap);

    cout << "  Output " << j << " points" << endl;

    return mm;

    //for(auto it : vector_index_adaptor<unsigned long,unsigned long>(Fmap,0U))
        //cout << it.get<0>() << ": " << it.get<1>() << endl;
}
