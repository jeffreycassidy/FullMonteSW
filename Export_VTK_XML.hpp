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
DOMNode* xml_createVTKTracer(DOMElement* el,const vector<array<float,3>>& points,const vector<unsigned long>& offsets);

DOMNode* xml_createVTKVolumeMesh(DOMDocument* doc,const TetraMesh* M,const vector<double>*data=NULL);

void xml_writeFile(string fn,DOMDocument* doc);

template<class Iterator>DOMNode* xml_writeCells(DOMElement* piece,Iterator begin,Iterator end);

template<class TetraIterator>DOMNode* xml_writeTetras(DOMElement* piece,TetraIterator begin,TetraIterator end);

template<class Iterator>DOMNode* xml_writePolys(DOMElement* piece,Iterator begin,Iterator end);
template<class Iterator>DOMNode* xml_writePoints(DOMElement* piece,Iterator begin,Iterator end);

template<class Iterator>DOMNode* xml_createVTKCellScalarData(DOMElement* el,string dataname,Iterator begin,Iterator end);

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

template<class Iterator>DOMNode* xml_createVTKCellScalarData(DOMElement* el,string dataname,Iterator begin,Iterator end)
{
	DOMDocument* doc = el->getOwnerDocument();

	DOMElement* celldata = doc->createElement(XMLAutoTranscoder("CellData"));
	celldata->setAttribute(XMLAutoTranscoder("Scalars"),XMLAutoTranscoder(dataname));

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
    //dataarray->setAttribute(XMLAutoTranscoder("NumberOfPoints"),XMLAutoTranscoder("%d",N));

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




/** Writes out a set of VTK Cells using an Iterator range.
 * If the Iterator gives sequence i_0 i_1 i_2 .. i_N, then connectivity is the N ranges [i_0,i_1) [i_1,i_2) ... [i_N-1,i_N)
 *
 * @param celltype VTK cell ID (VTK_VERTEX=1, VTK_POLY_VERTEX=2, VTK_LINE=3, VTK_POLY_LINE=4, VTK_TETRA=10
 * @param begin,end Iterator range for offsets
 *
 * @tparam Iterator Iterator which dereferences to the end offset of each cell
 */

template<class Iterator>DOMNode* xml_writeCells(DOMElement* piece,unsigned celltype,Iterator begin,Iterator end)
{
	DOMDocument* doc=piece->getOwnerDocument();

	// Here are the Lines
	DOMElement *cells = doc->createElement(XMLAutoTranscoder("Cells"));
	piece->appendChild(cells);

	// PolyData -> Piece -> Lines -> DataArray (offsets)
	DOMElement *cells_offs = doc->createElement(XMLAutoTranscoder("DataArray"));
	cells_offs->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("offsets"));
	cells_offs->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));
	unsigned long Nl=0,Np=0;

	{
		stringstream ss;
		ss << endl;
		for(auto it = begin; it != end; ++it,++Nl)
		{
			ss << *it;
			if (Nl % 10 == 9)
				ss << endl;
			else
				ss << ' ';
			Np=*it;
		}
		ss << endl;
		cells_offs->setTextContent(XMLAutoTranscoder(ss.str()));
		cells->appendChild(cells_offs);
	}

	// PolyData -> Piece -> Polys -> DataArray (connectivity)
	DOMElement *cells_conn = doc->createElement(XMLAutoTranscoder("DataArray"));
	cells_conn->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("connectivity"));
	cells_conn->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	{
		stringstream ss;
		ss << endl;
		for(unsigned long o=0; o<Np; ++o)
		{
			ss << o;
			if (o%10==9)
				ss << endl;
			else
				ss << ' ';
		}
		cells_conn->setTextContent(XMLAutoTranscoder(ss.str()));
	}
	cells->appendChild(cells_conn);

	// PolyData -> Piece -> Polys -> DataArray (connectivity)
	DOMElement *cells_types = doc->createElement(XMLAutoTranscoder("DataArray"));
	cells_types->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("types"));
	cells_types->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	{
		stringstream ss;
		ss << endl;
		for(unsigned long o=0; o<Nl; ++o)
		{
			ss << celltype;
			if (o%10==9)
				ss << endl;
			else
				ss << ' ';
		}
		cells_types->setTextContent(XMLAutoTranscoder(ss.str()));
	}
	cells->appendChild(cells_types);

	piece->setAttribute(XMLAutoTranscoder("NumberOfCells"),XMLAutoTranscoder("%d",Nl));

	return piece;
}


/** Writes out a set of VTK Tetras using an Iterator range.
 * Iterator must dereference to a TetraByPointID, and the corresponding point array must have been provided
 *
 * @tparam TetraIterator Iterator which dereferences to TetraByPointID
 */

template<class TetraIterator>DOMNode* xml_writeTetras(DOMElement* piece,TetraIterator begin,TetraIterator end)
{
	DOMDocument* doc=piece->getOwnerDocument();

	// Here are the Lines
	DOMElement *cells = doc->createElement(XMLAutoTranscoder("Cells"));
	piece->appendChild(cells);

	// CellData -> Piece -> Cells -> DataArray (connectivity)
	DOMElement *cells_conn = doc->createElement(XMLAutoTranscoder("DataArray"));
	cells_conn->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("connectivity"));
	cells_conn->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	unsigned long Nt=0;
	{
		stringstream ss;
		ss << endl;
		for(TetraIterator it=begin; it != end; ++it,++Nt)
			ss << (*it)[0]-1 << ' ' << (*it)[1]-1 << ' ' << (*it)[2]-1 << ' ' << (*it)[3]-1 << endl;
		cells_conn->setTextContent(XMLAutoTranscoder(ss.str()));
	}
	cells->appendChild(cells_conn);

	// CellData -> Piece -> Cells -> DataArray (offsets)
	DOMElement *cells_offs = doc->createElement(XMLAutoTranscoder("DataArray"));
	cells_offs->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("offsets"));
	cells_offs->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	{
		stringstream ss;
		ss << endl;
		for(unsigned i=0; i<Nt; ++i)
		{
			ss << (4*(i+1));
			if (Nt % 10 == 9)
				ss << endl;
			else
				ss << ' ';
		}
		ss << endl;
		cells_offs->setTextContent(XMLAutoTranscoder(ss.str()));
		cells->appendChild(cells_offs);
	}

	// CellData -> Piece -> Cells -> DataArray (connectivity)
	DOMElement *cells_types = doc->createElement(XMLAutoTranscoder("DataArray"));
	cells_types->setAttribute(XMLAutoTranscoder("Name"),XMLAutoTranscoder("types"));
	cells_types->setAttribute(XMLAutoTranscoder("type"),XMLAutoTranscoder("Int32"));

	{
		stringstream ss;
		ss << endl;
		for(unsigned long o=0; o<Nt; ++o)
		{
			ss << "10";			// VTK_TETRA=10
			if (o%10==9)
				ss << endl;
			else
				ss << ' ';
		}
		cells_types->setTextContent(XMLAutoTranscoder(ss.str()));
	}
	cells->appendChild(cells_types);

	piece->setAttribute(XMLAutoTranscoder("NumberOfCells"),XMLAutoTranscoder("%d",Nt));

	return piece;
}



/** Creates a DOM XML representation of a VTK tracer output from a points/offsets array
 *
 */

template<class PointIterator,class OffsetIterator>DOMNode* xml_createVTKTracer
	(DOMElement* el,PointIterator pointBegin,PointIterator pointEnd,OffsetIterator offsetBegin,OffsetIterator offsetEnd)
{
	DOMDocument* doc = el->getOwnerDocument();

    // PolyData section
    DOMElement* polydata = doc->createElement(XMLAutoTranscoder("UnstructuredGrid"));

    // PolyData -> Piece section
	DOMElement* piece=doc->createElement(XMLAutoTranscoder("Piece"));

	polydata->appendChild(piece);

	// write the points under this piece
	xml_writePoints(piece,pointBegin,pointEnd);

	xml_writeCells(piece,4,offsetBegin,offsetEnd);

	el->appendChild(polydata);

    return polydata;
}


/** Creates a DOM XML representation of a VTK tracer output from a points/offsets array
 *
 */

template<class PointIterator,class OffsetIterator>DOMNode* xml_createVTKPolyVertex
	(DOMElement* el,PointIterator pointBegin,PointIterator pointEnd,OffsetIterator offsetBegin,OffsetIterator offsetEnd)
{
	DOMDocument* doc = el->getOwnerDocument();

    // PolyData section
    DOMElement* polydata = doc->createElement(XMLAutoTranscoder("UnstructuredGrid"));

    // PolyData -> Piece section
	DOMElement* piece=doc->createElement(XMLAutoTranscoder("Piece"));

	polydata->appendChild(piece);

	// write the points under this piece
	xml_writePoints(piece,pointBegin,pointEnd);

	xml_writeCells(piece,2,offsetBegin,offsetEnd);

	el->appendChild(polydata);

    return polydata;
}

