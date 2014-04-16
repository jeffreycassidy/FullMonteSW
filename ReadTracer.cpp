#include <fstream>
#include <array>
#include <iostream>
#include <vector>

#include "Export_VTK_XML.hpp"
#include "TracerStep.hpp"

#include <boost/iterator/filter_iterator.hpp>

using namespace std;



/** Filter adaptor; makes any container T
 * @tparam T			Container type
 * @tparam Predicate	Predicate type; must have bool operator(const El&)
 */

template<class Predicate,class T>class filter_adaptor {
	T& obj;
	Predicate pred;
public:

	filter_adaptor(Predicate& pred_,T& obj_) : obj(obj_),pred(pred_){}

	typedef boost::filter_iterator<Predicate,typename T::const_iterator> 	const_iterator;
	typedef boost::filter_iterator<Predicate,typename T::iterator> 			iterator;

	iterator begin() { return boost::make_filter_iterator(pred,obj.begin(),obj.end()); }
	iterator end()   { return boost::make_filter_iterator(pred,obj.end());   }
};

template<class Predicate,class T>filter_adaptor<Predicate,T> make_filter_adaptor(Predicate pred_,T& obj_)
{
	return filter_adaptor<Predicate,T>(pred_,obj_);
}

int main(int argc,char **argv)
{
	ifstream is("tracer.0.bin");

	// get file size
	is.seekg(0,is.end);
	unsigned filesize = is.tellg();
	is.seekg(0,is.beg);

	// create data arrays
	vector<unsigned long> offsets;
	vector<array<float,3>> points(filesize/sizeof(TracerStep));
	vector<float> weights(filesize/sizeof(TracerStep));

	vector<TracerStep> steps(filesize/sizeof(TracerStep));
	is.read((char*)steps.data(),filesize);
	unsigned Nbytes=is.gcount();

	unsigned Npkt,Ne=0,pkt_age=0;

	vector<float>::iterator weights_end;
	vector<array<float,3>>::iterator points_end;


	// iterate through, pulling weights and point list ends for all direction changes
	weights_end=weights.begin();
	points_end=points.begin();

	for(const TracerStep& ts : make_filter_adaptor(TracerStep::IsDirectionChange,steps))
	{
		if (ts.event == TracerStep::Launch && pkt_age != 0)
		{
			offsets.push_back(Ne);
			cout << "Packet expired after " << pkt_age << " events" << endl;
			pkt_age=0;
			++Npkt;
		}
		else
			++pkt_age;
		*(points_end++)=ts.pos;
		*(weights_end++)=ts.weight;
		++Ne;
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
	xml_createVTKTracer(doc->getDocumentElement(),points.begin(),points_end,offsets.begin(),offsets.end());
	xml_writeFile("traces.xml",doc);
	doc->release();


	// iterate through, pulling weights and point list ends for all direction changes
	offsets.resize(0);
	points_end=points.begin();
	pkt_age=0;
	Ne=0;

	for(const TracerStep& ts : make_filter_adaptor(TracerStep::IsAccumulate,steps))
	{
		if (ts.event == TracerStep::Launch && pkt_age != 0)
		{
			offsets.push_back(Ne);
			cout << "Packet expired after " << pkt_age << " absorption events" << endl;
			pkt_age=0;
		}
		else
		{
			++pkt_age;
			*(points_end++)=ts.pos;
			++Ne;
		}
	}
	offsets.push_back(Ne);

	doc = xml_createVTKFileDoc();
	xml_createVTKPolyVertex(doc->getDocumentElement(),points.begin(),points_end,offsets.begin(),offsets.end());
	xml_writeFile("abs_exit.xml",doc);
	doc->release();




	// Now one more time for geometry reads
	offsets.resize(0);
	points_end=points.begin();
	pkt_age=0;
	Ne=0;

	for(const TracerStep& ts : make_filter_adaptor(TracerStep::IsGeometryRead,steps))
	{
		if (ts.event == TracerStep::Launch && pkt_age != 0)
		{
			offsets.push_back(Ne);
			cout << "Packet expired after " << pkt_age << " geometry reads" << endl;
			pkt_age=0;
		}
		else
		{
			++pkt_age;
			*(points_end++)=ts.pos;
			++Ne;
		}
	}
	offsets.push_back(Ne);

	doc = xml_createVTKFileDoc();
	xml_createVTKPolyVertex(doc->getDocumentElement(),points.begin(),points_end,offsets.begin(),offsets.end());
	xml_writeFile("geom_read.xml",doc);
	doc->release();
}
