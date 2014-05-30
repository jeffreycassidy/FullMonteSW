#include <boost/graph/adjacency_list.hpp>

#include <array>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkUnstructuredGrid.h>

class PointProperty {
public:
	array<double,3> coords;
	unsigned roi_id;
	unsigned curve_id;
	bool original;
	PointProperty(const array<double,3>& coords_=array<double,3>{0,0,0},unsigned roi_id_=0,unsigned curve_id_=0,bool original_=false) :
		coords(coords_),roi_id(roi_id_),curve_id(curve_id_),original(original_){}
};

class EdgeProperty {
public:
	unsigned roi_id=0;
	unsigned curve_id=0;
	bool original;
	EdgeProperty(unsigned roi_id_=0,unsigned curve_id_=0,bool original_=false) : roi_id(roi_id_),curve_id(curve_id_),original(original_){};
};


class TetraProperty {
public:
	array<unsigned,4> IDps;
	unsigned roi_id;
	TetraProperty(array<unsigned,4> IDps_=array<unsigned,4>{0,0,0,0},unsigned roi_id_=0) : IDps(IDps_),roi_id(roi_id_){}
};

class FaceProperty {
public:
	unsigned roi_id;
	FaceProperty(unsigned roi_id_=0) : roi_id(roi_id_){}
};

typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::undirectedS,
		PointProperty,								// points are vertices
		EdgeProperty,								// edges are, well, edges
		boost::no_property> PointGraph;



//typedef property<vertex_roi_id_t,unsigned> TROIProperty;
//typedef property<edge_roi_id_t,unsigned> FROIProperty;

typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::undirectedS,
		TetraProperty,								// tetras are vertices
		FaceProperty,								// faces are edges
		boost::no_property> TetraGraph;

class MeshGraph {
	PointGraph pg;
	TetraGraph tg;

	bool checkGraph(const PointGraph&,bool exc_=false);
	bool checkGraph(const TetraGraph&,bool exc_=false);

	void writeASCII_vertices(const string& fn,const PointGraph& pg) const;
	void writeASCII_edges(const string& fn,const PointGraph& pg) const;
	void writeASCII_vertices(const string& fn,const TetraGraph& tg) const;
	void writeASCII_edges(const string& fn,const TetraGraph& tg) const;

public:
	MeshGraph(const Pinnacle::File& pf,const vector<unsigned>& slices=vector<unsigned>(),const vector<unsigned>& rois=vector<unsigned>());

	vtkSmartPointer<vtkPoints> getVTKPoints() const;
	vtkSmartPointer<vtkCellArray> getVTKCurvePolys() const;
	vtkSmartPointer<vtkCellArray> getVTKMeshPolys() const;

	vtkSmartPointer<vtkUnstructuredGrid> getVTKMeshPolygons() const;

	vtkSmartPointer<vtkUnstructuredGrid> getVTKMeshTetras() const;

	vtkSmartPointer<vtkPolyData> getVTKCurveLines() const;
	vtkSmartPointer<vtkPolyData> getVTKMeshLines() const;

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
};


