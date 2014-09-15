#include "sse.hpp"
#include "fm-postgres/fm-postgres.hpp"
#include "fm-postgres/fmdbexportcase.hpp"
#include "graph.hpp"

#include <algorithm>

#include "Export_VTK_XML.hpp"
#include "MeshMapper.hpp"

#include "fluencemap.hpp"

MeshMapper listSurface(const TetraMesh* M,unsigned region);

void writeSurface(const TetraMesh* M,unsigned region)
{
	MeshMapper mm = listSurface(M,region);
	DOMDocument *doc = xml_createVTKFileDoc();
	DOMElement* el = doc->getDocumentElement();
	xml_createVTKSurfaceMesh(el,&mm);
	stringstream ss;
	ss << "output.surface" << region << ".xml";
	xml_writeFile(ss.str(),doc);
	doc->release();
}

int main(int argc,char **argv)
{
	// pull mesh from database
    boost::shared_ptr<PGConnection> dbconn=PGConnect();
    TetraMesh *M = exportMesh(*(dbconn.get()),1);

    // do the mapping
    MeshMapper mm = listSurface(M,0);

    // get surface fluence results
    FluenceMapBase *phi = exportResultSet(dbconn.get(),2887,1,M);
    vector<double> phi_v = phi->toVector(M->getNf());

    VolumeFluenceMap *phi_vol = (VolumeFluenceMap*)exportResultSet(dbconn.get(),2887,2,M);
    vector<double> phi_vv = phi_vol->toVector(M->getNt()+1);

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

	for(unsigned i=0;i<18;++i)
		writeSurface(M,i);

	doc = xml_createVTKFileDoc();
	xml_createVTKVolumeMesh(doc,M,&phi_vv);
	xml_writeFile("volume_mesh.xml",doc);
}

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
