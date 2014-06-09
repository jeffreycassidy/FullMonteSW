#include "PinnacleCurve.hpp"
#include "PinnacleROI.hpp"
#include "PinnacleObject.hpp"
#include "PinnacleFile.hpp"

#include "PinnacleGraph.hpp"

#include <vtkSmartPointer.h>


#include <boost/range.hpp>


#include <fstream>
#include <iostream>


using namespace std;

double distance_squared(const array<double,3>& a,const array<double,3>& b)
{
	double sum=0;
	for(unsigned i=0;i<3;++i)
	{
		double d=a[i]-b[i];
		sum += d*d;
	}
	return sum;
}

array<double,3> midpoint(const array<double,3>& a,const array<double,3>& b)
{
	array<double,3> mp;
	for(unsigned i=0;i<3;++i)
		mp[i]=0.5*(a[i]+b[i]);
	return mp;
}


double dot_product(const array<double,3>& a,const array<double,3>& b)
{
	double sum=0.0;
	for(unsigned i=0;i<3;++i)
		sum += a[i]*b[i];
	return sum;
}

template<class Graph>pair<typename Graph::edge_descriptor,bool> add_if_not_present(typename Graph::vertex_descriptor u,typename Graph::vertex_descriptor v,Graph& G)
{
	typename Graph::edge_descriptor e;
	bool found;

	tie(e,found)=edge(u,v,G);

	if(!found)
		return add_edge(u,v,G);

	return make_pair(e,false);
}

template<class Graph,class EdgeProperty>pair<typename Graph::edge_descriptor,bool> add_if_not_present(typename Graph::vertex_descriptor u,typename Graph::vertex_descriptor v,const EdgeProperty& ep,Graph& G)
{
	typename Graph::edge_descriptor e;
	bool found;

	tie(e,found)=edge(u,v,G);

	if(!found)
		return add_edge(u,v,ep,G);
	else
		return make_pair(e,false);
}



/** Gives the outwards-facing normal for two coplanar points in clockwise orientation.
 *
 */

array<double,3> outwards_normal2(const array<double,3>& v0,const array<double,3>& v1)
{
	array<double,3> dv,dv2;
	dv[0]=v1[0] - v0[0];
	dv[1]=v1[1] - v0[1];
	dv[2]=0;

	dv2[0]=dv[0]*dv[0];
	dv2[1]=dv[1]*dv[1];
	dv2[0]=0;

	//double k = 1.0/sqrtf(dv2[0]+dv2[1]);

	return array<double,3>{ -dv[1], dv[0], 0.0 };
	//return array<double,3> { 1.0,0.0,0.0 };
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
					//cout << "Curve of size " << c.size() << endl;
					Np += c.size()-1;
				}
				++IDc;
			}
		}
		++IDr;
	}

	IDr=IDc=0;
	os << "3" << endl;
	os << Np+1 << endl;

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
					array<double,3> p0,p_last;
					for(const array<double,3>& p : c.getPoints())
					{
						if (i < c.size()-1)
						{
							array<double,3> p_n_out = outwards_normal2( c.getPoints()[i==0 ? c.getNPoints()-2 : i-1],
									c.getPoints()[i+1]); 		// outwards normal for point

							v = add_vertex(PGPointProperty(p,IDr,IDc,c.getSliceID(),p_n_out,true),pg);
							//cout << "Vertex descriptor: " << v << endl;
							os << p[0] << ' ' << p[1] << ' ' << p[2] << endl;

							if (i>0)
							{
								add_edge(u,v,PGEdgeProperty(IDr,IDc,outwards_normal2(p_last,p),true),pg);			// rely on clockwise ordering
								//cout << "Point edge: " << u << "->" << v << endl;
								u=v;
							}
							else
							{
								u0 = u = v;		// set u0 to be start of loop
								p0=p;
							}
							p_last=p;
						}
						else
						{

							add_edge(u0,u,PGEdgeProperty(IDr,IDc,outwards_normal2(p0,p_last),true),pg);				// rely on clockwise ordering to get out-normal
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

	// dummy vertices top and bottom
	add_vertex(PGPointProperty(array<double,3>{ 0.0,0.0,200.0 },0,0,0,array<double,3>{0.0,0.0,0.0},true),pg);
	os << "0 0 200" << endl;
	//add_vertex(PointProperty(array<double,3>{ 0.0,0.0,0.0 },0,0,0,array<double,3>{0.0,0.0,0.0},false));
	//os << "0 0 0" << endl;

	os.close();

	system("qdelaunay Qt C-0 i < delaunay_in.txt > delaunay_out.txt");


	// load delaunay_out.txt to populate
	string ifn("delaunay_out.txt");
	ifstream is(ifn.c_str());

	unsigned Nt;
	array<PointGraph::vertex_descriptor,4> IDps;
	map<array<PointGraph::vertex_descriptor,4>,TetraGraph::vertex_descriptor> tets;
	multimap<array<PointGraph::vertex_descriptor,3>,TetraGraph::vertex_descriptor> face_tet_map;

	is >> Nt;
	is.ignore(1,'\n');
	for(unsigned i=0;i<Nt;++i)
	{
		string str;
		getline(is,str);
		stringstream ss(str);
		ss >> IDps[0] >> IDps[1] >> IDps[2] >> IDps[3];

		// internal data structure is 0-based with a null point in position 0; qdelaunay is 0-based without the null point passed
		++IDps[0];
		++IDps[1];
		++IDps[2];
		++IDps[3];

		// sort ascending for map matching
		sort(IDps.begin(),IDps.end());

		// add to map
		tets.insert(make_pair(IDps,0));

		// add to edge map if not already there
		add_if_not_present(IDps[0],IDps[1],pg);
		add_if_not_present(IDps[0],IDps[2],pg);
		add_if_not_present(IDps[0],IDps[3],pg);
		add_if_not_present(IDps[1],IDps[2],pg);
		add_if_not_present(IDps[1],IDps[3],pg);
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
	TetraGraph::vertex_descriptor null_tetra = add_vertex(tg);
	for(pair<const array<PointGraph::vertex_descriptor,4>,TetraGraph::vertex_descriptor>& p : tets)
	{
		p.second = add_vertex(TGTetraProperty(p.first),tg);

		// tetra points are already in ascending ID order, so faces below will be ordered too
		face_tet_map.insert( make_pair(array<PointGraph::vertex_descriptor,3>{ p.first[0],p.first[1],p.first[2] },p.second));
		face_tet_map.insert( make_pair(array<PointGraph::vertex_descriptor,3>{ p.first[0],p.first[1],p.first[3] },p.second));
		face_tet_map.insert( make_pair(array<PointGraph::vertex_descriptor,3>{ p.first[0],p.first[2],p.first[3] },p.second));
		face_tet_map.insert( make_pair(array<PointGraph::vertex_descriptor,3>{ p.first[1],p.first[2],p.first[3] },p.second));
	}

	cout << "Creating tetra connectivity graph (|face_tet_map|=" << face_tet_map.size() << ")" << endl;

	// map faces to tetras
	for(multimap<array<PointGraph::vertex_descriptor,3>,TetraGraph::vertex_descriptor>::const_iterator it=face_tet_map.cbegin(); it != face_tet_map.cend(); )
	{
		pair<array<PointGraph::vertex_descriptor,3>,TetraGraph::vertex_descriptor> u = *it;

		if (++it == face_tet_map.end())
			break;

		pair<array<PointGraph::vertex_descriptor,3>,TetraGraph::vertex_descriptor> v = *it;

		TetraGraph::edge_descriptor e;
		bool added;
		if (u.first==v.first)
		{
			//cout << "  " << u.second << "->" << v.second << endl;
			++it;
			tie(e,added) = add_if_not_present(u.second,v.second,TGFaceProperty(u.first),tg);			// has a neighbour; add edge
		}
		else
		{																	// outer face; add edge to null tetra
			tie(e,added) = add_if_not_present(u.second,null_tetra,TGFaceProperty(u.first),tg);
//			cout << "  singleton node " << u.first << " (followed by " << v.first << ')'<< endl;
		}

		bool found;
		PointGraph::edge_descriptor pe;

		tie(pe,found) = edge(it->first[0],it->first[1],pg);
		if (!found)
		{
			cerr << "not found" << endl;
			tg[e].pg_edges[0]=TetraGraph::edge_descriptor();
		}
		else
			tg[e].pg_edges[0]=pe;

		tie(pe,found) = edge(it->first[0],it->first[2],pg);

		if (!found)
		{
			cerr << "not found" << endl;
		tg[e].pg_edges[1]=TetraGraph::edge_descriptor();
		}
		else
			tg[e].pg_edges[1]=pe;

		tie(pe,found) = edge(it->first[1],it->first[2],pg);
		if (!found)
		{
			cerr << "not found" << endl;
			tg[e].pg_edges[2]=TetraGraph::edge_descriptor();
		}
		else
			tg[e].pg_edges[2]=pe;
	}

	os.close();

	os.open("face_tet_map.out");

	// check face connectivity histogram

	array<unsigned,4> fhist={0,0,0,0};
	for(multimap<array<PointGraph::vertex_descriptor,3>,TetraGraph::vertex_descriptor>::const_iterator it=face_tet_map.begin(); it != face_tet_map.end(); )
	{
		array<PointGraph::vertex_descriptor,3> F=it->first;
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



	///////////////////////////////////////////////////////////////////////////
	// Create FaceGraph

	defineBoundaries();

	// Doing this externally because making it an internal graph property causes a circular type dependency
	for(TetraGraph::edge_descriptor e : edges(tg))
		for(unsigned i=0;i<3;++i)
			edge_face_map.insert(make_pair(tg[e].pg_edges[i],e));

	typedef multimap<PointGraph::edge_descriptor,TetraGraph::edge_descriptor>::const_iterator mmit;
	mmit lb,ub;
	map<unsigned,unsigned> ehist;

	for(lb = edge_face_map.cbegin(), ub=edge_face_map.upper_bound(lb->first);
			lb != edge_face_map.cend();
			lb=ub,ub=edge_face_map.upper_bound(lb->first))
	{
		unsigned i;
		for(i=0; lb != ub; ++lb,++i){}
		ehist[i]++;
	}

	cout << "Edge connectivity histogram: " << endl;
	for(const pair<unsigned,unsigned>& p : ehist)
		cout << "  " << p.first << ": " << p.second << endl;
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

	cout << "Tetra connectivity histogram:" << endl;
	for(unsigned i=0;i<5;++i)
		cout << "  " << i << ": " << idhist[i] << endl;
	cout << "  >4: 0" << endl;
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
	os << "#   <x> <y> <z> <ROI> <curve> <slice>" << endl;
	os << num_vertices(pg) << endl;
	auto r = vertices(pg);
	for(auto it = r.first; it != r.second; ++it)
		os << pg[*it].coords << ' ' << pg[*it].roi_id << ' ' << pg[*it].curve_id << ' ' << pg[*it].slice_id << endl;
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
		vtkIdType IDps[4] = { tg[*v].pg_points[0], tg[*v].pg_points[1], tg[*v].pg_points[2], tg[*v].pg_points[3] };
		if (IDps[0] && IDps[1] && IDps[2] && IDps[3])
			tets->InsertNextCell(VTK_TETRA,4,IDps);
	}

	tets->SetPoints(getVTKPoints());

	return tets;
}


vtkSmartPointer<vtkPolyData> MeshGraph::getVTKMeshLines() const
{
	vtkSmartPointer<vtkPolyData> data = vtkPolyData::New();

	data->SetPoints(getVTKPoints());
	data->SetLines(getVTKMeshPolys());
	return data;
}


/** Return an array of tetra types. *
 *
 * Values:
 * 0		Unknown/unassigned
 * 128		Tetra on boundary (degree < 4)
 */

vtkSmartPointer<vtkUnstructuredGrid> MeshGraph::getVTKTetraData_Types() const
{
	vtkSmartPointer<vtkUnstructuredGrid> tets = getVTKMeshTetras();
	vtkCellData *cd=tets->GetCellData();

	unsigned Np = num_vertices(tg);

	vtkSmartPointer<vtkUnsignedCharArray> ia = vtkUnsignedCharArray::New();
	ia->SetNumberOfComponents(1);
	ia->SetNumberOfTuples(Np);
	ia->SetName("Tetra Types");

	unsigned Nbound=0,Nother=0;

	auto p = vertices(tg);
	for(auto v = p.first; v != p.second; ++v)
	{
		if(out_degree(*v,tg) < 4)
		{
			ia->SetTuple1(*v,1);
			++Nbound;
		}
		else {
			ia->SetTuple1(*v,0);
			++Nother;
		}
	}

	cout << "Found following tetra types:" << endl;
	cout << "  Boundary: " << Nbound << endl;
	cout << "  Other:    " << Nother << endl;

	cd->SetScalars(ia);
	cout << "Active scalars set to " << cd->SetActiveScalars("Tetra Types") << endl;

	return tets;
}

unsigned MeshGraph::incident_boundary_faces_to_edge(PointGraph::edge_descriptor e) const
{
	unsigned n=0;

	auto __p = edge_face_map.equal_range(e);

	for(auto p = __p.first; p != __p.second; ++p)
		if (tg[p->second].isBoundary)
			++n;

	return n;
}

unsigned MeshGraph::incident_boundary_faces(TetraGraph::edge_descriptor e) const
{
	unsigned n=0,Nfb;

	for(unsigned i=0;i<3;++i)
		n += incident_boundary_faces_to_edge(tg[e].pg_edges[i])-1;
	return n;
}


// loop over all faces (edges) in the TetraGraph (Delaunay-triangulation tetra connectivity)
// add faces where following criteria are met:
//	1) 2 points coplanar
//  2) 2 coplanar points adjacent in original Delaunay
//	3) All refer to same ROI
//	4) other point +/- 1 slice
//	5) normals pointing in similar direction (dot product > 0)

void MeshGraph::defineBoundaries()
{
	for(TetraGraph::edge_descriptor e : edges(tg))
	{
		tg[e].isBoundary=false;
		array<pair<unsigned,PointGraph::vertex_descriptor>,3> p;			// { sliceID, pointID } for each point

		for(unsigned i=0;i<3;++i)
			p[i] = make_pair(pg[tg[e].pg_points[i]].slice_id,tg[e].pg_points[i]);

		// ensure all 3 refer to same ROI
		if (pg[tg[e].pg_points[0]].roi_id != pg[tg[e].pg_points[1]].roi_id || pg[tg[e].pg_points[1]].roi_id != pg[tg[e].pg_points[2]].roi_id)
			continue;

		// check 2 points coplanar, put their indices into first two elements and other into third
		if (p[0].first == p[1].first){}
		else if (p[1].first==p[2].first)
			swap(p[0],p[2]);
		else if (p[0].first==p[2].first)
			swap(p[1],p[2]);
		else
			continue;

		// check other point +/- 1 slice
		if (abs((int)(p[1].first-p[2].first))!=1)
			continue;

		bool found;
		PointGraph::edge_descriptor e_base;

		tie(e_base,found)=edge(p[1].second,p[0].second,pg);
		assert(found);

		if (!pg[e_base].original)
			continue;

		// check that outward normals are in similar direction
		if (dot_product(pg[e_base].n_out,pg[p[2].second].n_out) < 0)
			continue;

		tg[e].isBoundary=true;

		// edge->face map
		array<PointGraph::vertex_descriptor,3> IDps = { p[0].second, p[1].second, p[2].second };
		sort(IDps.begin(),IDps.end());
	}
}

/** Yet another try at solving this problem, this time using information gleaned from outward-facing normals.
 * Seems promising!
 */

vtkSmartPointer<vtkPolyData> MeshGraph::vtkRibbonFromCurves2(const Pinnacle::File& f) const
{
	vtkSmartPointer<vtkPolyData> polys = vtkPolyData::New();
	vtkSmartPointer<vtkCellArray> cells = vtkCellArray::New();

	vtkSmartPointer<vtkDoubleArray> normalarray = vtkDoubleArray::New();

	vtkSmartPointer<vtkUnsignedCharArray> faceconn = vtkUnsignedCharArray::New();

	polys->SetPolys(cells);

	// save point normals
	normalarray->SetNumberOfComponents(3);
	auto p = vertices(pg);
	for(auto v=p.first; v != p.second; ++v)
		normalarray->InsertNextTuple(pg[*v].n_out.data());

	polys->GetPointData()->SetVectors(normalarray);

	// save the points
	polys->SetPoints(getVTKPoints());

	map<unsigned,unsigned> hist,ehist;

	auto pe = edges(tg);
	unsigned Nb=0;
	for(auto e=pe.first; e != pe.second; ++e)
	{
		if (tg[*e].isBoundary)
		{
			++Nb;
			unsigned N_adjacent_faces = incident_boundary_faces(*e);
			polys->InsertNextCell(VTK_TRIANGLE,3,array<vtkIdType,3>{ tg[*e].pg_points[0], tg[*e].pg_points[1], tg[*e].pg_points[2] }.data());
			faceconn->InsertNextTuple1(N_adjacent_faces);
			hist[N_adjacent_faces]++;
		}
	}

	//cout << "Found " << Nb << " boundary faces" << endl;

//	cout << "Edge connectivity histogram" << endl;
//	for(const pair<unsigned,unsigned>& p : ehist)
//		cout << "  " << p.first << ": " << p.second << endl;


//	cout << "Face connectivity histogram" << endl;
//	for(const pair<unsigned,unsigned>& p : hist)
//		cout << "  " << p.first << ": " << p.second << endl;

	polys->GetCellData()->SetScalars(faceconn);

	return polys;
}
