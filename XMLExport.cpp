/// XMLExport sends data to VTK for viewing
#include "sse.hpp"
#include "newgeom.hpp"
#include "graph.hpp"
#include "fm-postgres/fm-postgres.hpp"

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>

#include <list>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#include <xercesc/dom/DOMNodeIterator.hpp>
#include <xercesc/dom/DOMNodeFilter.hpp>


using namespace xercesc;
using namespace std;

class XMLStringTranscoder {
	list<XMLCh*> p_list;

	char tmp[256];          // used for sprintf call using variadic arguments

public:

	const XMLCh* operator()(string str_)	{ XMLCh* ch=XMLString::transcode(str_.c_str()); p_list.push_back(ch); return ch; };
	const XMLCh* operator()(const char* s_) { XMLCh* ch=XMLString::transcode(s_); 			p_list.push_back(ch); return ch; }
	template<typename... Args>const XMLCh* operator()(const char* fmt_,const Args&... args)
		{ XMLCh* ch; sprintf(tmp,fmt_,args...); p_list.push_back(ch=XMLString::transcode(tmp)); return ch; }

	~XMLStringTranscoder(){ for(list<XMLCh*>::iterator it=p_list.begin(); it != p_list.end(); ++it) XMLString::release(&(*it)); }
};

DOMDocument* xml_createDoc();
DOMNode* xml_createMesh(DOMDocument* doc,const TetraMesh* M);

int main(int argc,char **argv)
{
    boost::shared_ptr<PGConnection> dbconn = PGConnect();
    TetraMesh *M = exportMesh(*dbconn.get(),1);
    XMLStringTranscoder transcoder;
	try {
		XMLPlatformUtils::Initialize();
	}
	catch(...){
		cerr << "Exception loading Xerces-C++ platform" << endl;
		return 1;
	}

    // get the implementation
	XMLCh *tmp = XMLString::transcode("LS");
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tmp);
	XMLString::release(&tmp);


    // Create output document DOM model
    DOMDocument* doc = xml_createDoc();
    DOMElement* el = doc->getDocumentElement();
    DOMNode* M_node = xml_createMesh(doc,M);

    el->appendChild(M_node);

    // Display to stdout & write to file
	cout << "=================================================" << endl;
	cout << "New XML file:" << endl;

	DOMLSSerializer* ls = ((DOMImplementationLS*)impl)->createLSSerializer();
	DOMConfiguration* cfg = ls->getDomConfig();

	if(cfg->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint,true))
		cfg->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint,true);

	DOMLSOutput* o = ((DOMImplementationLS*)impl)->createLSOutput();

	XMLFormatTarget* myFmtTarget = new StdOutFormatTarget();
    XMLFormatTarget* fileFmtTarget = new LocalFileFormatTarget(transcoder("output.xml"));

	try{
//	    o->setByteStream(myFmtTarget);
//		ls->write(doc,o);
        o->setByteStream(fileFmtTarget);
        ls->write(doc,o);
	}
	catch(const XMLException& e){
		char *msg = XMLString::transcode(e.getMessage());
		XMLString::release(&msg);
	}

//	delete myFmtTarget;
//    delete fileFmtTarget;
	doc->release();

//	XMLPlatformUtils::Terminate();
}


DOMDocument* xml_createDoc()
{
    XMLStringTranscoder transcoder;
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(transcoder("LS"));

	DOMDocument *doc = impl->createDocument(0,transcoder("VTKFile"),0);

	DOMElement* el = doc->getDocumentElement();
	el->setAttribute(transcoder("type"),transcoder("VTKFile"));
    el->setAttribute(transcoder("version"),transcoder("0.1"));
//    el->setAttribute(transcoder("byte_order"),transcoder("little_endian"));

	DOMNode* comm=doc->createComment(transcoder("Created by XMLExport v0.01"));
	el->appendChild(comm);

    return doc;
}


DOMNode* xml_createMesh(DOMDocument* doc,const TetraMesh* M)
{
	XMLStringTranscoder transcoder;

    unsigned Np=M->getNp();
    unsigned Nt=M->getNt();
    unsigned Nf=M->getNf();

    // PolyData section
    DOMElement* polydata = doc->createElement(transcoder("PolyData"));

    // PolyData -> Piece section
	DOMElement* piece=doc->createElement(transcoder("Piece"));

	piece->setAttribute(transcoder("NumberOfPoints"),transcoder("%d",Np));
	piece->setAttribute(transcoder("NumberOfPolys"),transcoder("%d",Nf));
	piece->setAttribute(transcoder("NumberOfVerts"),transcoder("0"));
	piece->setAttribute(transcoder("NumberOfLines"),transcoder("0"));
	piece->setAttribute(transcoder("NumberOfStrips"),transcoder("0"));
	polydata->appendChild(piece);

	// PolyData -> Piece -> Points      | holds coordinates for each point
	DOMElement* pts=doc->createElement(transcoder("Points"));
	piece->appendChild(pts);

	DOMElement *pts_data=doc->createElement(transcoder("DataArray"));
	pts_data->setAttribute(transcoder("NumberOfComponents"),transcoder("%d",3));
	pts_data->setAttribute(transcoder("type"),transcoder("Float32"));
	pts_data->setAttribute(transcoder("format"),transcoder("ascii"));

    // Transcode a whole lot of floats
    {
        stringstream ss;
        ss << endl;

        for(auto it = M->pointBegin(); it != M->pointEnd(); ++it)
            ss << (*it)[0] << ' ' << (*it)[1] << ' ' << (*it)[2] << endl;

        XMLCh* xmls = XMLString::transcode(ss.str().c_str());
        pts_data->setTextContent(xmls);
    }
	pts->appendChild(pts_data);


    // Here are the polygons
	DOMElement *polys = doc->createElement(transcoder("Polys"));
	piece->appendChild(polys);

	// PolyData -> Piece -> Polys -> DataArray (connectivity)
	DOMElement *polys_conn = doc->createElement(transcoder("DataArray"));
	polys_conn->setAttribute(transcoder("Name"),transcoder("connectivity"));
	polys_conn->setAttribute(transcoder("type"),transcoder("Int32"));
	{
		stringstream ss;
        ss << endl;
        unsigned i=0;
        for(auto it = M->faceIDBegin(); it != M->faceIDEnd(); ++it,++i)
        {
			ss << (*it)[0]-1 << ' ' << (*it)[1]-1 << ' ' << (*it)[2]-1;
            if (i%10==9)
                ss << endl;
            else
                ss << ' ';
        }
		polys_conn->setTextContent(transcoder(ss.str()));
	}
	polys->appendChild(polys_conn);


	// PolyData -> Piece -> Polys -> DataArray (offsets)
	DOMElement *polys_offs = doc->createElement(transcoder("DataArray"));
	polys_offs->setAttribute(transcoder("Name"),transcoder("offsets"));
	polys_offs->setAttribute(transcoder("type"),transcoder("Int32"));

    {
    	stringstream ss;
        ss << endl;
    	for(unsigned i=0;i<Nf;++i)		// offset points to end of index-list for each element
        {
	    	ss << 3*i+3;
            if (i%10==9)
                ss << endl;
            else
                ss << ' ';
        }
        ss << endl;
    	polys_offs->setTextContent(transcoder(ss.str()));
    
	    polys->appendChild(polys_offs);
    }

    return polydata;
}
