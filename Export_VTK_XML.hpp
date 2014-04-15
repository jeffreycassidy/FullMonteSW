#include "newgeom.hpp"
#include "graph.hpp"
#include "MeshMapper.hpp"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/LocalFileFormatTarget.hpp>

using namespace xercesc;

DOMDocument* xml_createVTKFileDoc();
DOMNode* xml_createVTKSurfaceMesh(DOMElement* doc,const TetraMesh* M);
DOMNode* xml_createVTKSurfaceMesh(DOMElement* el,const MeshMapper* M,const vector<double>* data=NULL);

void xml_writeFile(string fn,DOMDocument* doc);

template<class Iterator>DOMNode* xml_writePolys(DOMElement* piece,Iterator begin,Iterator end);
template<class Iterator>DOMNode* xml_writePoints(DOMElement* piece,Iterator begin,Iterator end);

template<class Iterator>DOMNode* xml_createVTKSurfaceScalarData(DOMElement* el,string dataname,Iterator begin,Iterator end);

class GlobalXMLState {
	bool initialized;

public:
	GlobalXMLState(){
	    // Initialize the platform
		try {
			XMLPlatformUtils::Initialize();
			cout << "Xerces-C++ initialized properly" << endl;
			initialized=true;
		}
		catch(...){
			cerr << "Exception loading Xerces-C++ platform" << endl;
			initialized=false;
		}
	}

	~GlobalXMLState(){
		cout << "Xerces-C++ shut down properly" << endl;
		XMLPlatformUtils::Terminate(); }
};




/** Xerces XML transcoder with automatic memory management.
 * Calls XMLString::transcode to code the argument into XML, then XMLString::release to destroy
 */

class XMLAutoTranscoder {
	XMLCh *p;
public:
	XMLAutoTranscoder(string str_)		: p(XMLString::transcode(str_.c_str())){}
	XMLAutoTranscoder(const char* s_) 	: p(XMLString::transcode(s_)){}
	template<typename... Args>XMLAutoTranscoder(const char* fmt_,const Args&... args)		// note: possible buffer overflow
			{ char tmp[256]; sprintf(tmp,fmt_,args...); p=XMLString::transcode(tmp); }

	~XMLAutoTranscoder(){ if (p) XMLString::release(&p); }

	operator const XMLCh*() const { return p; }

	XMLAutoTranscoder(const XMLAutoTranscoder&) = delete;
	XMLAutoTranscoder(XMLAutoTranscoder&& t){ p=t.p; t.p=NULL; }
};




/** Writes out the points between the iterators [begin,end).
 *
 * @param piece Parent DOMNode, must be a VTK "Piece" tag
 * @param begin,end	Iterator range
 * @tparam Iterator	Iterator type which dereferences such that (*it)[0]..(*it)[2] are valid coordinates
 */

template<class Iterator>DOMNode* xml_writePoints(DOMElement* piece,Iterator begin,Iterator end)
{
	DOMDocument *doc=piece->getOwnerDocument();

	// PolyData -> Piece -> Points      | holds coordinates for each point
	DOMElement* pts=doc->createElement(XMLAutoTranscoder("Points"));

	DOMElement *pts_data=doc->createElement(XMLAutoTranscoder("DataArray"));
	pts_data->setAttribute(XMLAutoTranscoder("NumberOfComponents"),XMLAutoTranscoder("%d",3));
	pts_data->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Float32"));
	pts_data->setAttribute(XMLAutoTranscoder("format"),XMLAutoTranscoder("ascii"));

    unsigned Np=0;

    // Transcode a whole lot of floats
    {
        stringstream ss;
        ss << endl;

        for(auto it=begin; it != end; ++it,++Np)
            ss << (*it)[0] << ' ' << (*it)[1] << ' ' << (*it)[2] << endl;

        XMLCh* xmls = XMLString::transcode(ss.str().c_str());
        pts_data->setTextContent(xmls);
    }
	piece->setAttribute(XMLAutoTranscoder("NumberOfPoints"),XMLAutoTranscoder("%d",Np));
	piece->appendChild(pts);
	pts->appendChild(pts_data);
	return piece;
}



/** Creates a DOM XML representation of a VTK surface scalar array in Iterator range [begin,end)
 *
 */

template<class Iterator>DOMNode* xml_createVTKSurfaceScalarData(DOMElement* el,string dataname,Iterator begin,Iterator end)
{
	DOMDocument* doc = el->getOwnerDocument();

	DOMElement* celldata = doc->createElement(XMLAutoTranscoder("CellData"));
	celldata->setAttribute(XMLAutoTranscoder("Scalars"),XMLAutoTranscoder("Emittance"));

	el->appendChild(celldata);

    // PolyData section
    DOMElement* dataarray = doc->createElement(XMLAutoTranscoder("DataArray"));
    dataarray->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder(dataname));

    dataarray->setAttribute(XMLAutoTranscoder("NumberOfComponents"),XMLAutoTranscoder("%d",1));
    dataarray->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Float32"));
    dataarray->setAttribute(XMLAutoTranscoder("format"),XMLAutoTranscoder("ascii"));

    unsigned N=0;

    // Transcode a whole lot of floats
    {
    	stringstream ss;
    	ss << endl;

    	for(auto it=begin; it != end; ++it,++N)
    		ss << (*it) << endl;

    	XMLCh* xmls = XMLString::transcode(ss.str().c_str());
    	dataarray->setTextContent(xmls);
    }
    dataarray->setAttribute(XMLAutoTranscoder("NumberOfPoints"),XMLAutoTranscoder("%d",N));

    celldata->appendChild(dataarray);

    return dataarray;
}




/** Write out the Faces (triangular polygons/VTK "polys") between iterators [begin,end).
 *
 * @param piece	DOMElement pointing to the VTK "Piece" tag
 * @param begin,end Iterator range
 * @return Pointer to the created DOMNode
 */

template<class Iterator>DOMNode* xml_writePolys(DOMElement* piece,Iterator begin,Iterator end)
{
	DOMDocument* doc=piece->getOwnerDocument();

	// Here are the polygons
	DOMElement *polys = doc->createElement(XMLAutoTranscoder("Polys"));
	piece->appendChild(polys);

	// PolyData -> Piece -> Polys -> DataArray (connectivity)
	DOMElement *polys_conn = doc->createElement(XMLAutoTranscoder("DataArray"));
	polys_conn->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("connectivity"));
	polys_conn->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	unsigned Nf=0;
	{
		stringstream ss;
		ss << endl;
		for(auto it = begin; it != end; ++it,++Nf)
		{
			ss << (*it)[0]-1 << ' ' << (*it)[1]-1 << ' ' << (*it)[2]-1;		// VTK uses 0-based, while FullMonte internals are 1-based
			if (Nf%10==9)
				ss << endl;
			else
				ss << ' ';
		}
		polys_conn->setTextContent(XMLAutoTranscoder(ss.str()));
	}
	polys->appendChild(polys_conn);


	// PolyData -> Piece -> Polys -> DataArray (offsets)
	DOMElement *polys_offs = doc->createElement(XMLAutoTranscoder("DataArray"));
	polys_offs->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("offsets"));
	polys_offs->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	{
		stringstream ss;
		ss << endl;
		for(unsigned i=0;i<Nf;++i)		// offset points to end of index-list for each element
											// gotcha: VTK says index is to end of list; actually it's to start of next!!
		{
			ss << 3*i+3;
			if (i%10==9)
				ss << endl;
			else
				ss << ' ';
		}
		ss << endl;
		polys_offs->setTextContent(XMLAutoTranscoder(ss.str()));

		polys->appendChild(polys_offs);
	}
	piece->setAttribute(XMLAutoTranscoder("NumberOfPolys"),XMLAutoTranscoder("%d",Nf));

	return piece;
}
