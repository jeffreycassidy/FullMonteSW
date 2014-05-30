#include "PinnacleCurve.hpp"
#include "PinnacleROI.hpp"
#include "PinnacleObject.hpp"
#include "PinnacleFile.hpp"

#include "PinnacleGraph.hpp"

#include <vtkSmartPointer.h>

#include <vtkPoints.h>
#include <vtkCellArray.h>

#include <boost/range.hpp>

#include <fstream>
#include <iostream>


using namespace std;

template<class Graph>pair<typename Graph::edge_descriptor,bool> add_if_not_present(typename Graph::vertex_descriptor u,typename Graph::vertex_descriptor v,Graph& G)
{
	if(!edge(u,v,G).second)
		return add_edge(u,v,G);

	return make_pair(typename Graph::edge_descriptor(),false);
}

template<class Graph,class EdgeProperty>pair<typename Graph::edge_descriptor,bool> add_if_not_present(typename Graph::vertex_descriptor u,typename Graph::vertex_descriptor v,const EdgeProperty& ep,Graph& G)
{
	if(!edge(u,v,G).second)
		return add_edge(u,v,ep,G);
	else
		return make_pair(0,false);
}


MeshGraph::MeshGraph(const Pinnacle::File& pf,const vector<unsigned>& slices,const vector<unsigned>& rois)
{

	ofstream os("delaunay_in.txt");

	unsigned IDr=0,IDc=0,Np=0;
	for(const Pinnacle::ROI& r : pf.getROIs())
	{
		if (rois.size() == 0 || binary_search(rois.begin(),rois.end(),IDr))
		{
			IDc=0;
			for(const Pinnacle::Curve& c : r.getCurves())
			{
				if (slices.size()==0 || binary_search(slices.begin(),slices.end(),c.getSliceID()))
				{
					// this curve is inside the slice
					cout << "Curve of size " << c.size() << endl;
					Np += c.size()-1;
				}
				++IDc;
			}
		}
		++IDr;
	}

	// filters out a single z slice
	IDr=IDc=0;
	os << "3" << endl;
	os << Np << endl;

	// add null vertex for point zero
	add_vertex(pg);

	PointGraph::vertex_descriptor u0,u,v;
	for(const Pinnacle::ROI& r : pf.getROIs())
	{
		if (rois.size()==0 || binary_search(rois.begin(),rois.end(),IDr))
		{
			IDc=0;
			for(const Pinnacle::Curve& c : r.getCurves())
			{
				if (slices.size()==0 || binary_search(slices.begin(),slices.end(),c.getSliceID()))
				{
					unsigned i=0;
					for(const array<double,3>& p : c.getPoints())
					{
						if (i < c.size()-1)
						{
							v = add_vertex(PointProperty(p,IDr,IDc,true),pg);
							//cout << "Vertex descriptor: " << v << endl;
							os << p[0] << ' ' << p[1] << ' ' << p[2] << endl;

							if (i>0)
							{
								add_edge(u,v,EdgeProperty(IDr,IDc,true),pg);
								//cout << "Point edge: " << u << "->" << v << endl;
								u=v;
							}
							else
								u0 = u = v;		// set u0 to be start of loop
						}
						else
						{
							add_edge(u0,u,EdgeProperty(IDr,IDc,true),pg);
							//cout << "Point edge: " << u << "->" << u0 << " to close loop" << endl;
						}
						++i;
					}
				}
				++IDc;
			}
		}
		++IDr;

	}

	system("qdelaunay Qt i < delaunay_in.txt > delaunay_out.txt");


	// load delaunay_out.txt to populate
	string ifn("delaunay_out.txt");
	ifstream is(ifn.c_str());

	unsigned Nt;
	array<unsigned,4> IDps;
	map<array<unsigned,4>,TetraGraph::vertex_descriptor> tets;
	multimap<array<unsigned,3>,TetraGraph::vertex_descriptor> face_tet_map;

	is >> Nt;
	is.ignore(1,'\n');
	for(unsigned i=0;i<Nt;++i)
	{
		string str;
		getline(is,str);
		stringstream ss(str);
		ss >> IDps[0] >> IDps[1] >> IDps[2] >> IDps[3];

		++IDps[0];
		++IDps[1];
		++IDps[2];
		++IDps[3];

		//cout << "Read: " << IDps << endl;

		// sort ascending for map matching
		sort(IDps.begin(),IDps.end());

		// add to map
		//if(IDps[0] && IDps[1] && IDps[2] && IDps[3])
			tets.insert(make_pair(IDps,0));

		// add to edge map if not already there
		/*if(IDps[0])
		{
			if (IDps[1])*/
				add_if_not_present(IDps[0],IDps[1],pg);

			//if (IDps[2])
				add_if_not_present(IDps[0],IDps[2],pg);

			//if (IDps[3])
				add_if_not_present(IDps[0],IDps[3],pg);
		/*}
		if (IDps[1])
		{
			if(IDps[2])*/
				add_if_not_present(IDps[1],IDps[2],pg);

			//if(IDps[3])
				add_if_not_present(IDps[1],IDps[3],pg);
		//}
		//if (IDps[2] && IDps[3])
			add_if_not_present(IDps[2],IDps[3],pg);

		// check for failed conversion
		if (ss.fail())
		{
			cerr << "Failed while reading " << ifn << ": \"" << str << '"' << endl;
			throw string("Exception reading file");
		}

		// check for trailing characters
		while(!ss.eof() && ss.peek()==' ')
			ss.ignore(1);

		if (!ss.eof())
		{
			cerr << "Extra characters while reading " << ifn << ": \"" << str << '"' << endl;
			throw string("Exception reading file");
		}
	}

	cout << "Read " << Nt << " tetras from " << ifn << endl;
	cout << "Creating face-tetra map" << endl;

	// null vertex at ID=0
	add_vertex(tg);
	for(pair<const array<unsigned,4>,TetraGraph::vertex_descriptor>& p : tets)
	{
		p.second = add_vertex(TetraProperty(p.first),tg);

		// tetra points are already in ascending ID order, so faces below will be ordered too
		face_tet_map.insert( make_pair(array<unsigned,3>{ p.first[0],p.first[1],p.first[2] },p.second));
		face_tet_map.insert( make_pair(array<unsigned,3>{ p.first[0],p.first[1],p.first[3] },p.second));
		face_tet_map.insert( make_pair(array<unsigned,3>{ p.first[0],p.first[2],p.first[3] },p.second));
		face_tet_map.insert( make_pair(array<unsigned,3>{ p.first[1],p.first[2],p.first[3] },p.second));
	}

	cout << "Creating tetra connectivity graph (|face_tet_map|=" << face_tet_map.size() << ")" << endl;

	// map faces to tetras
	for(multimap<array<unsigned,3>,TetraGraph::vertex_descriptor>::const_iterator it=face_tet_map.begin(); it != face_tet_map.end(); )
	{
		pair<array<unsigned,3>,TetraGraph::vertex_descriptor> u = *it;

		if (++it == face_tet_map.end())
			break;

		pair<array<unsigned,3>,TetraGraph::vertex_descriptor> v = *it;

		if (u.first==v.first)
		{
			//cout << "  " << u.second << "->" << v.second << endl;
			++it;
			add_edge(u.second,v.second,tg);
		}
		else
		{
//			cout << "  singleton node " << u.first << " (followed by " << v.first << ')'<< endl;
		}
	}

	os.close();

	os.open("face_tet_map.out");


	array<unsigned,4> fhist={0,0,0,0};

	for(multimap<array<unsigned,3>,TetraGraph::vertex_descriptor>::const_iterator it=face_tet_map.begin(); it != face_tet_map.end(); )
	{
		array<unsigned,3> F=it->first;
		unsigned i;

		for(i=0; it != face_tet_map.end() && it->first==F; ++i,++it)
			os << it->second << ' ';
		os << endl;

		if (i > 2)
		{
			cerr << "ERROR: More than 2 tetras joined by face " << endl;
			fhist[3]++;
		}
		else
			fhist[i]++;
	}

	cout << "Face connectivity histogram" << endl;

	for(unsigned i=0;i<4;++i)
		cout << "  " << i << ": " << fhist[i] << endl;

	checkGraph(tg);
	checkGraph(pg);

	writeASCII_vertices("tri.points.out",pg);
	writeASCII_edges("tri.edges.out",pg);
	writeASCII_vertices("tri.faces.out",tg);
	writeASCII_edges("tri.tetras.out",tg);
}

bool MeshGraph::checkGraph(const TetraGraph& tg,bool exc_)
{
	bool isOK=true;
	array<unsigned,5> idhist={0,0,0,0,0};
	array<unsigned,5> odhist={0,0,0,0,0};

	auto p = vertices(tg);

	if (p.first == p.second)
		return true;

	TetraGraph::vertex_descriptor v=*p.first;

	for(auto it=p.first; it != p.second; v=*(++it))
	{
		unsigned ind=in_degree(v,tg);
		unsigned outd=out_degree(v,tg);
		if(ind < 5)
			++idhist[ind];
		else
		{
			cerr << "ERROR: Invalid in-degree " << ind << " on TetraGraph vertex " << v << endl;
			isOK=false;
			if (exc_)
				throw string("");
		}

		if(outd < 5)
			++odhist[outd];
		else
		{
			cerr << "ERROR: Invalid out-degree " << outd << " on TetraGraph vertex " << v << endl;
			isOK=false;
			if (exc_)
				throw string("");
		}
	}
	return isOK;
}

bool MeshGraph::checkGraph(const PointGraph& pg,bool exc_)
{
	return true;
}

void MeshGraph::writeASCII_vertices(const string& fn,const PointGraph& pg) const
{
	ofstream os("tri.points.out");
	os << "# Point graph output" << endl;
	os << "# <Nverts> followed by Nverts lines of" << endl;
	os << "#   <x> <y> <z> <ROI> <curve>" << endl;
	os << num_vertices(pg) << endl;
	auto r = vertices(pg);
	for(auto it = r.first; it != r.second; ++it)
		os << pg[*it].coords << ' ' << pg[*it].roi_id << ' ' << pg[*it].curve_id << endl;
}

void MeshGraph::writeASCII_edges(const string& fn,const PointGraph& pg) const
{
	ofstream os(fn.c_str());
	os << "# Graph edges output" << endl;
	os << "# <Nedges> followed by Nedges lines of" << endl;
	os << "#   <u> <v> <ROI> <curve> <original>" << endl;
	os << "# where <original>==1 iff this edge was defined in a curve" << endl;
	os << num_edges(pg) << endl;
	auto r = edges(pg);
	for(auto it = r.first; it != r.second; ++it)
		os << source(*it,pg) << ' ' << target(*it,pg) << ' ' << pg[*it].roi_id << ' ' << pg[*it].curve_id << ' ' << pg[*it].original << endl;
}


void MeshGraph::writeASCII_vertices(const string& fn,const TetraGraph& tg) const
{
	ofstream os(fn.c_str());
	os << "# Face connectivity output" << endl;
	os << "# <tetID> <tetID>" << endl;
}

void MeshGraph::writeASCII_edges(const string& fn,const TetraGraph& tg) const
{
}


vtkSmartPointer<vtkPoints> MeshGraph::getVTKPoints() const
{
	vtkSmartPointer<vtkPoints> pts = vtkPoints::New();

	unsigned Np=num_vertices(pg);

	if (Np < 2)
	{
		cerr << "ERROR: Too few points (<=2) in getVTKPoints" << endl;
		throw string("ERROR: Too few points (<=2) in getVTKPoints");
	}

	pts->SetDataTypeToDouble();
	pts->SetNumberOfPoints(Np);

	// copy to array
	auto p=vertices(pg);
	unsigned i=0;
	for(auto v=p.first; v!=p.second;++v,++i)
		pts->SetPoint(i,pg[*v].coords[0],pg[*v].coords[1],pg[*v].coords[2]);

	cout << "Added " << i << " points to the mesh" << endl;

	return pts;
}

vtkSmartPointer<vtkCellArray> MeshGraph::getVTKCurvePolys() const
{
	vtkSmartPointer<vtkCellArray> lines = vtkCellArray::New();

	unsigned Nl=0;
	for(auto e : edges(pg))
		if (pg[e].original)
		{
			vtkIdType ids[2]={source(e,pg),target(e,pg)};
			lines->InsertNextCell(2, ids);
			++Nl;
		}

	cout << "getVTKCurvePolys: Added " << Nl << " lines to output mesh" << endl;

	return lines;
}

vtkSmartPointer<vtkCellArray> MeshGraph::getVTKMeshPolys() const
{
	vtkSmartPointer<vtkCellArray> lines = vtkCellArray::New();

	unsigned Nl=0;
	for(auto e : edges(pg))
	{
		PointGraph::vertex_descriptor u=source(e,pg),v=target(e,pg);
		if (u != 0 && v != 0)
		{
			vtkIdType ids[2]={u,v};
			lines->InsertNextCell(2, ids);
			++Nl;
		}
	}

	cout << "getVTKMeshPolys: Added " << Nl << " lines to output mesh" << endl;

	return lines;
}

vtkSmartPointer<vtkPolyData> MeshGraph::getVTKCurveLines() const
{
	vtkSmartPointer<vtkPolyData> data = vtkPolyData::New();

	data->SetPoints(getVTKPoints());
	data->SetLines(getVTKCurvePolys());
	return data;
}

vtkSmartPointer<vtkUnstructuredGrid> MeshGraph::getVTKMeshTetras() const
{
	vtkSmartPointer<vtkUnstructuredGrid> tets = vtkUnstructuredGrid::New();

	auto p = vertices(tg);

	for(auto v = p.first; v != p.second; ++v)		// loop over all tetras, adding to output
	{
		vtkIdType IDps[4] = { tg[*v].IDps[0], tg[*v].IDps[1], tg[*v].IDps[2], tg[*v].IDps[3] };
		if (IDps[0] && IDps[1] && IDps[2] && IDps[3])
			tets->InsertNextCell(VTK_TETRA,4,IDps);
	}

	tets->SetPoints(getVTKPoints());

	return tets;
}

/*vtkSmartPointer<vtkUnstructuredGrid> MeshGraph::getVTKMeshPolygons() const
{
	vtkSmartPointer<vtkUnstructuredGrid> polys = vtkUnstructuredGrid::New();
}*/

vtkSmartPointer<vtkPolyData> MeshGraph::getVTKMeshLines() const
{
	vtkSmartPointer<vtkPolyData> data = vtkPolyData::New();

	data->SetPoints(getVTKPoints());
	data->SetLines(getVTKMeshPolys());
	return data;
}
