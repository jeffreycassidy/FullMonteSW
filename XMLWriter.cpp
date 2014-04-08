#include "graph.hpp"

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <sstream>
#include <iostream>
#include <string>
#include <stdio.h>

#include <list>

// Insert linebreaks into 72-character Base64 encoding

typedef boost::archive::iterators::insert_linebreaks<
		boost::archive::iterators::base64_from_binary<
			boost::archive::iterators::transform_width<const char*,6,8> >,
		72> boost_base64;

/// Should use XML attribute encoding="base64"
/// <nodetype name="" encoding="base64"> DATA </nodetype>

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

class NullFilter : public DOMNodeFilter {
public:
	virtual FilterAction acceptNode(const DOMNode*) const { return FILTER_ACCEPT; }
};

class TempXMLString {
	XMLCh* ch;

public:
	TempXMLString(string str_) : ch(XMLString::transcode(str_.c_str())){};
	TempXMLString(const char* s_) : ch(XMLString::transcode(s_)){}
	TempXMLString() : ch(NULL){}

	~TempXMLString(){ XMLString::release(&ch); }

	operator const XMLCh*() const { return ch; }
};


void writePolyData(XMLFormatTarget* fmtTarget);

int main(int argc,char **argv)
{
	try {
		XMLPlatformUtils::Initialize();
	}
	catch(...){
		cerr << "Exception loading Xerces-C++ platform" << endl;
		return 1;
	}

	XMLCh *tmp = XMLString::transcode("LS");
	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(tmp);
	DOMLSSerializer* ls = ((DOMImplementationLS*)impl)->createLSSerializer();

	DOMConfiguration* cfg = ls->getDomConfig();

	if(cfg->canSetParameter(XMLUni::fgDOMWRTFormatPrettyPrint,true))
		cfg->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint,true);

	XMLString::release(&tmp);

	tmp = XMLString::transcode("hello");
	DOMDocument *doc = impl->createDocument(0,tmp,0);
	XMLString::release(&tmp);


	XMLFormatTarget* myFmtTarget = new StdOutFormatTarget();
	DOMLSOutput* op = ((DOMImplementationLS*)impl)->createLSOutput();
	op->setByteStream(myFmtTarget);

    XMLFormatTarget* fileFmtTarget = new LocalFileFormatTarget(TempXMLString("output.xml"));

	DOMElement* el = doc->getDocumentElement();

	tmp = XMLString::transcode("Created by FOOBAR");
	DOMNode* comm=doc->createComment(tmp);
	el->appendChild(comm);
	XMLString::release(&tmp);

	tmp = XMLString::transcode("fubar");
	DOMNode* n = doc->createElement(tmp);
	el->appendChild(n);
	XMLString::release(&tmp);

	if(!doc)
		cerr << "Error: null document" << endl;

	try{
		ls->write(doc,op);
	}
	catch(const XMLException& e){
		char *msg = XMLString::transcode(e.getMessage());
		XMLString::release(&msg);
	}


	cout << "Now messing around with Base64 coding" << endl;

	char tmpchar[256];

	for(unsigned i=0;i<256;++i)
		tmpchar[i]=0;

	for(boost_base64 it(tmpchar); it != boost_base64(tmpchar+256); ++it)
		cout << *it;

	cout << "=================================================" << endl;
	cout << "New XML file:" << endl;

	writePolyData(myFmtTarget);

    writePolyData(fileFmtTarget);

	doc->release();

	XMLPlatformUtils::Terminate();
}


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

void writePolyData(XMLFormatTarget* fmtTarget)
{
    unsigned Nt=256;
    unsigned Np=1024;
	XMLStringTranscoder transcoder;

	// File-dependent parts
	const XMLCh* vtktype = transcoder("PolyData");

	DOMImplementation *impl = DOMImplementationRegistry::getDOMImplementation(transcoder("LS"));
	DOMLSSerializer* ls = ((DOMImplementationLS*)impl)->createLSSerializer();

	DOMDocument *doc = impl->createDocument(0,transcoder("VTKFile"),0);

	DOMLSOutput* op = ((DOMImplementationLS*)impl)->createLSOutput();
	op->setByteStream(fmtTarget);

	DOMElement* el = doc->getDocumentElement();
	el->setAttribute(transcoder("type"),vtktype);
    el->setAttribute(transcoder("version"),transcoder("0.1"));
//    el->setAttribute(transcoder("byte_order"),transcoder("little_endian"));

    DOMElement* polydata = doc->createElement(vtktype);
    el->appendChild(polydata);

	DOMElement* piece=doc->createElement(transcoder("Piece"));

	piece->setAttribute(transcoder("NumberOfPoints"),transcoder("%d",Np));
	piece->setAttribute(transcoder("NumberOfPolys"),transcoder("%d",Nt));
	piece->setAttribute(transcoder("NumberOfVerts"),transcoder("0"));
	piece->setAttribute(transcoder("NumberOfLines"),transcoder("0"));
	piece->setAttribute(transcoder("NumberOfStrips"),transcoder("0"));
	polydata->appendChild(piece);


	// Point data (coordinates)
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
        for(unsigned i=0;i<Np;++i)
            ss << (0.1+i) << ' ' << (0.2+i) << ' ' << (0.3+i) << endl;
    
        XMLCh* xmls = XMLString::transcode(ss.str().c_str());
        pts_data->setTextContent(xmls);
    }
	pts->appendChild(pts_data);


    // Here are the polygons
	DOMElement *polys = doc->createElement(transcoder("Polys"));
	piece->appendChild(polys);

	// PolyData -> Polys -> DataArray(connectivity)
	DOMElement *polys_conn = doc->createElement(transcoder("DataArray"));
	polys_conn->setAttribute(transcoder("Name"),transcoder("connectivity"));
	polys_conn->setAttribute(transcoder("type"),transcoder("Int32"));
	{
		stringstream ss;
        ss << endl;
		for(unsigned i=0;i<Nt;++i)
        {
			ss << 10*i << ' ' << 10*i+1 << ' ' << 10*i+2 << ' ' << 10*i+3;
            if (i%10==9)
                ss << endl;
            else
                ss << ' ';
        }
		polys_conn->setTextContent(transcoder(ss.str()));
	}
	polys->appendChild(polys_conn);

	// PolyData -> Polys -> DataArray(offsets)
	DOMElement *polys_offs = doc->createElement(transcoder("DataArray"));
	polys_offs->setAttribute(transcoder("Name"),transcoder("offsets"));
	polys_offs->setAttribute(transcoder("type"),transcoder("Int32"));

    {
    	stringstream ss;
        ss << endl;
    	for(unsigned i=0;i<Nt;++i)		// offset points to end of index-list for each element
        {
	    	ss << 4*i+3;
            if (i%10==9)
                ss << endl;
            else
                ss << ' ';
        }
        ss << endl;
    	polys_offs->setTextContent(transcoder(ss.str()));
    
	    polys->appendChild(polys_offs);
    }

	// CellData types: VTK_TRIANGLE=5, VTK_TETRA=10, VTK_POLY_LINE=4
/*	DOMElement *cell_data = doc->createElement(transcoder("CellData"));
	cell_data->setAttribute(transcoder("Scalars"),transcoder("Emittance"));		// specifies the active data set
	piece->appendChild(cell_data);

	DOMElement *data_emittance = doc->createElement(transcoder("DataArray"));
	data_emittance->setAttribute(transcoder("Name"),transcoder("Emittance"));
	cell_data->appendChild(data_emittance);

	DOMNode* n = doc->createElement(vtktype);
	el->appendChild(n);

	if(!doc)
		cerr << "Error: null document" << endl;
*/
	try{
		ls->write(doc,op);
	}
	catch(const XMLException& e){
		char *msg = XMLString::transcode(e.getMessage());
		XMLString::release(&msg);
	}

	delete fmtTarget;
	doc->release();
}

