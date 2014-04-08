#include "Export_VTK_XML.hpp"

#include <boost/iterator/permutation_iterator.hpp>

GlobalXMLState xerces_platform;


/** Creates a DOMDocument for the top level of the VTK File */

DOMDocument* xml_createVTKFileDoc()
{
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(XMLAutoTranscoder("LS"));

	DOMDocument *doc = impl->createDocument(0,XMLAutoTranscoder("VTKFile"),0);

	DOMElement* el = doc->getDocumentElement();
	el->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("VTKFile"));
    el->setAttribute(XMLAutoTranscoder("version"),XMLAutoTranscoder("0.1"));
//    el->setAttribute(transcoder("byte_order"),transcoder("little_endian"));

	DOMNode* comm=doc->createComment(XMLAutoTranscoder("Created by XMLExport v0.01"));
	el->appendChild(comm);

    return doc;
}


/** Creates a DOM XML representation of a VTK surface mesh from TetraMesh M.
 *
 */

DOMNode* xml_createVTKSurfaceMesh(DOMElement* el,const TetraMesh* M)
{
	DOMDocument* doc = el->getOwnerDocument();

    // PolyData section
    DOMElement* polydata = doc->createElement(XMLAutoTranscoder("PolyData"));

    // PolyData -> Piece section
	DOMElement* piece=doc->createElement(XMLAutoTranscoder("Piece"));

	//piece->setAttribute(transcoder("NumberOfVerts"),transcoder("0"));
	//piece->setAttribute(transcoder("NumberOfLines"),transcoder("0"));
	//piece->setAttribute(transcoder("NumberOfStrips"),transcoder("0"));
	polydata->appendChild(piece);

	// write the points under this piece
	xml_writePoints(piece,M->pointBegin(),M->pointEnd());

	// write the polydata (triangular faces) now
	xml_writePolys(piece,M->faceIDBegin(),M->faceIDEnd());

	el->appendChild(polydata);

    return polydata;
}




DOMNode* xml_createVTKSurfaceMesh(DOMElement* el,const MeshMapper* M,const vector<double>* data)
{
	DOMDocument* doc = el->getOwnerDocument();

	// PolyData section
	DOMElement* polydata = doc->createElement(XMLAutoTranscoder("PolyData"));

	// PolyData -> Piece section
	DOMElement* piece=doc->createElement(XMLAutoTranscoder("Piece"));
	polydata->appendChild(piece);

	// write the points under this piece
	xml_writePoints(piece,
			make_transform_iterator(M->points.begin(),GetPayload<unsigned long,Point<3,double>>),
			make_transform_iterator(M->points.end(),GetPayload<unsigned long,Point<3,double>>));

	// write the polydata (triangular faces) now
	xml_writePolys(piece,
			make_transform_iterator(M->faces.begin(),GetPayload<unsigned long,FaceByPointID>),
			make_transform_iterator(M->faces.end(),GetPayload<unsigned long,FaceByPointID>));

	if (data)
	{
		// get the permutation
		vector<unsigned long> perm = M->getFacePermutation();

		// check size
		cout << "Point permutation has size " << perm.size() << endl;

		// write it
		xml_createVTKSurfaceScalarData(piece,"Emittance",
			boost::make_permutation_iterator(data->begin(),perm.begin()),
			boost::make_permutation_iterator(data->end()  ,perm.end())  );
	}

	el->appendChild(polydata);

	return polydata;
}



/** Writes a DOMDocument out to a file
 * @param fn File name
 * @param doc DOM document
 *
 * Enables pretty printing.
 */

void xml_writeFile(string fn,DOMDocument* doc)
{
	cout << "Writing VTK XML output to " << fn << endl;
    // get the implementation
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(XMLAutoTranscoder("LS"));
	DOMLSSerializer* ls = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMConfiguration* cfg = ls->getDomConfig();

	if(cfg->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint,true))
		cfg->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint,true);

	DOMLSOutput* o = ((DOMImplementationLS*)impl)->createLSOutput();

	XMLFormatTarget* fileFmtTarget = new LocalFileFormatTarget(XMLAutoTranscoder(fn));

	try{
		o->setByteStream(fileFmtTarget);
		ls->write(doc,o);
	}
	catch(const XMLException& e){
		char *msg = XMLString::transcode(e.getMessage());
		XMLString::release(&msg);
	}

	cout << "  Done" << endl;

	delete fileFmtTarget;
}
