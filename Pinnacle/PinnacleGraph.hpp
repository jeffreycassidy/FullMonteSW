#include <boost/graph/adjacency_list.hpp>

#include <array>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellData.h>

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkUnsignedCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/graph/adj_list_serialize.hpp>

class PGPointProperty {
public:
	array<double,3> coords;
	unsigned roi_id;
	unsigned curve_id;
	unsigned slice_id=0;
	array<double,3> n_out=array<double,3>{1.0,0.0,0.0};			// outwards-facing normal (unit vector) within slice plane
	bool original;
	PGPointProperty(const array<double,3>& coords_=array<double,3>{0,0,0},
			unsigned roi_id_=0, unsigned curve_id_=0, unsigned slice_id_=0,
			const array<double,3>& n_out_=array<double,3>{1.0,0.0,0.0},
			bool original_=false) :
		coords(coords_),roi_id(roi_id_),curve_id(curve_id_),slice_id(slice_id_),n_out(n_out_),original(original_){}

	template<class Archive>void serialize(Archive& ar,const unsigned ver){
		ar & coords & roi_id & curve_id & slice_id & n_out & original;
	}
};

class PGEdgeProperty {
public:
	unsigned roi_id=0;
	unsigned curve_id=0;
	array<double,3> n_out=array<double,3>{1.0,0.0,0.0};			// outwards-facing normal (unit vector) within slice plane
	bool original;
	PGEdgeProperty(unsigned roi_id_=0,unsigned curve_id_=0,
			const array<double,3>& n_out_=array<double,3>{ 1.0,0.0,0.0},
			bool original_=false) : roi_id(roi_id_),curve_id(curve_id_),n_out(n_out_),original(original_){};


	template<class Archive>void serialize(Archive& ar,const unsigned ver){
		ar & roi_id & curve_id & n_out & original;
	}
};

typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::undirectedS,
		PGPointProperty,								// points are vertices
		PGEdgeProperty,								// edges are, well, edges
		boost::no_property> PointGraph;


class TGTetraProperty {
public:
	array<PointGraph::vertex_descriptor,4> pg_points;
	unsigned roi_id;
	TGTetraProperty(array<PointGraph::vertex_descriptor,4> pg_points_=array<PointGraph::vertex_descriptor,4>{0,0,0,0}
		,unsigned roi_id_=0)
		: pg_points(pg_points_),roi_id(roi_id_){}


	template<class Archive>void serialize(Archive& ar,const unsigned ver){
		ar & pg_points & roi_id;
	}
};

class TGFaceProperty {
public:
	array<PointGraph::vertex_descriptor,3> pg_points;
	array<PointGraph::edge_descriptor,3>   pg_edges;
	bool isBoundary=false;
	TGFaceProperty(const array<PointGraph::vertex_descriptor,3>& pg_points_=array<PointGraph::vertex_descriptor,3>(),
			const array<PointGraph::edge_descriptor,3>& pg_edges_= array<PointGraph::edge_descriptor,3>())
		: pg_points(pg_points_),pg_edges(pg_edges_){}

	template<class Archive>void serialize(Archive& ar,const unsigned ver){
		ar & pg_points & pg_edges;
	}
};

typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::undirectedS,
		TGTetraProperty,								// tetras are vertices
		TGFaceProperty,								// faces are edges
		boost::no_property> TetraGraph;



class MeshGraph {
	PointGraph pg;
	TetraGraph tg;

	multimap<PointGraph::edge_descriptor,TetraGraph::edge_descriptor> edge_face_map;

	bool checkGraph(const PointGraph&,bool exc_=false);
	bool checkGraph(const TetraGraph&,bool exc_=false);

	void writeASCII_vertices(const string& fn,const PointGraph& pg) const;
	void writeASCII_edges(const string& fn,const PointGraph& pg) const;
	void writeASCII_vertices(const string& fn,const TetraGraph& tg) const;
	void writeASCII_edges(const string& fn,const TetraGraph& tg) const;

	template<class Archive>void serialize(Archive& ar,const unsigned version)
	{
		ar & pg & tg & edge_face_map;
	}

public:
	MeshGraph(const Pinnacle::File& pf,const vector<unsigned>& slices=vector<unsigned>(),const vector<unsigned>& rois=vector<unsigned>());

	vtkSmartPointer<vtkPoints> getVTKPoints() const;
	vtkSmartPointer<vtkCellArray> getVTKCurvePolys() const;
	vtkSmartPointer<vtkCellArray> getVTKMeshPolys() const;

	vtkSmartPointer<vtkUnstructuredGrid> getVTKMeshPolygons() const;

	vtkSmartPointer<vtkUnstructuredGrid> getVTKMeshTetras() const;

	vtkSmartPointer<vtkUnstructuredGrid> getVTKTetraData_Types() const;

	vtkSmartPointer<vtkPolyData> getVTKCurveLines() const;
	vtkSmartPointer<vtkPolyData> getVTKMeshLines() const;

	vtkSmartPointer<vtkPolyData> getVTKMeshFaces() const;

	void defineBoundaries();

	vtkSmartPointer<vtkPolyData> vtkRibbonFromCurves2(const Pinnacle::File& f) const;

	unsigned incident_boundary_faces(TetraGraph::edge_descriptor e) const;
	unsigned incident_boundary_faces_to_edge(PointGraph::edge_descriptor e) const;

	bool ExcludeZero(PointGraph::edge_descriptor e) const { return source(e,pg) != 0 && target(e,pg) != 0; }

	unsigned checkSelfEdges() const {
		unsigned Nse=0;
		for(PointGraph::edge_descriptor e : edges(pg))
			if (source(e,pg)==target(e,pg))
				++Nse;
		return Nse;
	}

	// displays orphan edges
	bool OrphanEdgesOnly(PointGraph::edge_descriptor e) const {
		return NumIncidentBoundaryFaces(e)==1;
	}

	// return the number of boundary faces incident on an edge
	unsigned NumIncidentBoundaryFaces(PointGraph::edge_descriptor e) const {
		auto r = edge_face_map.equal_range(e);
		unsigned i=0;

		// count number of incident boundary faces
		for(auto it=r.first; it != r.second; ++it)
			i += tg[it->second].isBoundary;
		return i;
	}

	bool isBoundary(TetraGraph::edge_descriptor e) const {
		return tg[e].isBoundary;
	}

	template<class IncludePred,class ScalarFunction>vtkSmartPointer<vtkPolyData> filterEdges(IncludePred ip,ScalarFunction f) const;
	template<class IncludePred,class ScalarFunction>vtkSmartPointer<vtkPolyData> filterFaces(IncludePred ip,ScalarFunction f) const;

	void writeAll(string fn_root) const
	{
		stringstream ss(fn_root);
		ss << fn_root << ".points.out" << endl;
		writeASCII_vertices(ss.str(),pg);

		ss.clear();
		ss.str(fn_root);
		ss << ".mesh.out" << endl;
		writeASCII_edges(ss.str(),pg);

		ss.clear();
		ss.str(fn_root);
		ss << fn_root << ".tetras.out" << endl;
		writeASCII_vertices(ss.str(),tg);

		ss.clear();
		ss.str(fn_root);
		ss << fn_root << ".faces.out" << endl;
		writeASCII_edges(ss.str(),tg);
	}

	friend boost::serialization::access;
};

/**
 *
 * @tparam IncludePred		Predicate taking a PointGraph::edge_descriptor which returns true if the edge is to be included.
 * @tparam ScalarFunction	Scalar function giving the data value to be assigned to the edge.
 */

template<class IncludePred,class ScalarFunction>vtkSmartPointer<vtkPolyData> MeshGraph::filterEdges(IncludePred ip,ScalarFunction f) const
{
	vtkSmartPointer<vtkPolyData> lines = vtkPolyData::New();
	vtkSmartPointer<vtkUnsignedCharArray> linedata = vtkUnsignedCharArray::New();

	lines->SetPoints(getVTKPoints());

	lines->GetCellData()->SetScalars(linedata);
	lines->SetLines(vtkCellArray::New());

	for(PointGraph::edge_descriptor e : edges(pg))
	{
		if (ip(e))
		{
			lines->InsertNextCell(VTK_LINE,2,array<vtkIdType,2>{ source(e,pg), target(e,pg) }.data());
			linedata->InsertNextTuple1(f(e));
		}
	}

	return lines;
}


/** Yet another try at solving this problem, this time using information gleaned from outward-facing normals.
 * Seems promising!
 */

template<class IncludePred,class ScalarFunction>vtkSmartPointer<vtkPolyData> MeshGraph::filterFaces(IncludePred ip,ScalarFunction f) const
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

	// map<unsigned,unsigned> hist,ehist;

	auto pe = edges(tg);
	unsigned Nb=0;
	for(auto e=pe.first; e != pe.second; ++e)
	{
		if (ip(*e))
		{
//		if (tg[*e].isBoundary)
		//{
			++Nb;
			//unsigned N_adjacent_faces = incident_boundary_faces(*e);
			polys->InsertNextCell(VTK_TRIANGLE,3,array<vtkIdType,3>{ tg[*e].pg_points[0], tg[*e].pg_points[1], tg[*e].pg_points[2] }.data());
			faceconn->InsertNextTuple1(f(*e));
			//hist[N_adjacent_faces]++;
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


