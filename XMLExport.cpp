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

#include "Export_VTK_XML.hpp"

using namespace xercesc;
using namespace std;

int main(int argc,char **argv)
{
	// load a test mesh (Digimouse) from database
    boost::shared_ptr<PGConnection> dbconn = PGConnect();
    TetraMesh *M = exportMesh(*dbconn.get(),1);

    // Create output document DOM model
    DOMDocument* doc = xml_createVTKFileDoc();
    DOMElement* el = doc->getDocumentElement();
    xml_createVTKSurfaceMesh(el,M);

	// Used for writing to stdout
	//XMLFormatTarget* myFmtTarget = new StdOutFormatTarget();

	xml_writeFile("output.xml",doc);
	doc->release();
}
