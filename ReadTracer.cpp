#include <fstream>
#include <array>
#include <iostream>
#include <vector>

#include "Export_VTK_XML.hpp"

using namespace std;

typedef struct TracerStep_t {
    char event;
    float weight;
    array<float,3> pos;
    float t;
} TracerStep;

int main(int argc,char **argv)
{
	ifstream is("tracer.0.bin");
	TracerStep ts;

	double w_last=1;

	// get file size
	is.seekg(0,is.end);
	unsigned filesize = is.tellg();
	is.seekg(0,is.beg);

	// create data arrays
	vector<unsigned long> offsets;
	vector<array<float,3>> points(filesize/sizeof(TracerStep));
	vector<float> weights(filesize/sizeof(TracerStep));

	// read first
	is.read((char*)&ts,sizeof(TracerStep));

	unsigned Nbytes,Npkt,Ne,pkt_age=0;

	for(Npkt=Ne=0,Nbytes=is.gcount(); !is.eof(); ++Ne)
	{
		//cout << "Event " << (unsigned)ts.event << " weight " << ts.weight << " pos (" << ts.pos[0] << "," << ts.pos[1] << "," << ts.pos[2] << ") t" << ts.t << endl;
		if (ts.weight==1 && w_last != 1)
		{
			offsets.push_back(Ne);
			cout << "Packet expired after " << pkt_age << " events" << endl;
			pkt_age=1;
			++Npkt;
		}
		else
			++pkt_age;
		w_last=ts.weight;
		points[Ne]=ts.pos;
		weights[Ne]=ts.weight;
		is.read((char*)&ts,sizeof(TracerStep));
		Nbytes += is.gcount();
	}
	offsets.push_back(Ne);

	cout << "File size: " << filesize << endl;
	cout << "  Total bytes read: " << Nbytes << endl;
	cout << "  sizeof(TracerStep): " << sizeof(TracerStep) << endl;
	cout << "  TracerSteps: "  << Nbytes/sizeof(TracerStep) << endl;

	cout << "Trace totals" << endl;
	cout << "  Packets: " << offsets.size() << endl;
	cout << "  Events:  " << points.size() << endl;

	DOMDocument* doc = xml_createVTKFileDoc();
	xml_createVTKTracer(doc->getDocumentElement(),points,offsets);
	xml_writeFile("traces.xml",doc);

	doc->release();

}
