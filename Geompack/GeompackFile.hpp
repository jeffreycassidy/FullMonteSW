#include <string>
#include <array>
#include <vector>

#include <boost/iterator/transform_iterator.hpp>
#include <functional>

using namespace std;

/** Represents a Geompack++ mesh as defined in Sec 7 "3-D Mesh File Format" of Geompack TR ZCS2008-01 Mar 2008.
 *
 * Uses 1-based indices.
 *
 */

class GeompackMesh {
public:
	typedef array<double,3> Point;

	typedef struct {
		array<unsigned,4> v;
		unsigned regcode;
		array<unsigned,4> facinfo;

		typedef array<unsigned,4>::const_iterator const_iterator;
		typedef unsigned value;
		typedef unsigned& reference;
		typedef unsigned* pointer;

		typedef long difference_type;
		typedef forward_iterator_tag iterator_category;

		pair<const_iterator,const_iterator> getVertices() const { return make_pair(v.cbegin(),v.cend()); }

		const_iterator begin() const { return v.cbegin(); }
		const_iterator   end() const { return v.cend();   }
	} Tetra;


private:
	typedef struct {
		unsigned nodecode;
		unsigned icsur;
		double ucsur;
		double vcsur;
	} VertexInfo;


	typedef struct {
		unsigned edgv0,edgv1;
		unsigned edginfo;
	} ConstrainedEdge;

	vector<Point> P;
	vector<unsigned> P_info;

	vector<VertexInfo> info;
	vector<Tetra> tetinfo;
	vector<ConstrainedEdge> ce;

	static array<unsigned,4> GetVTKPointIndices(Tetra& t){
		array<unsigned,4> v=t.v;
		for(unsigned& vi : v)
			vi -= 1;
		return v;
	}

public:

	GeompackMesh(string fn_,bool verbose=false){ load(fn_,verbose); };

	void load(string fn,bool verbose=false);

	bool checkIntegrity(bool printStderr=false) const;

	class Exception {
	public:
		string message() const;
	};

	class BadElementType : public GeompackMesh::Exception {
		unsigned expected,received;
	public:
		BadElementType(unsigned expected_,unsigned received_) : expected(expected_),received(received_){}
		string message() const;
	};

	// Need to subtract one!!
	//static auto MinusOne = std::bind(std::minus<unsigned>(),std::placeholders::_1,1);

	typedef decltype(boost::make_transform_iterator(array<unsigned,4>().cbegin(),std::bind(std::minus<unsigned>(),std::placeholders::_1,1U))) zerobased_iterator;

	static pair<zerobased_iterator,zerobased_iterator> GetZerobasedVertices(const Tetra& t){
		return make_pair(boost::make_transform_iterator(t.v.cbegin(),std::bind(std::minus<unsigned>(),std::placeholders::_1,1U)),
				boost::make_transform_iterator(t.v.cend(),std::bind(std::minus<unsigned>(),std::placeholders::_1,1U)));
	}

	static pair<array<unsigned,4>::const_iterator,array<unsigned,4>::const_iterator> GetVertices(const Tetra& t) {
		return make_pair(t.v.cbegin(),t.v.cend());
	}

	typedef vector<Point>::const_iterator VTKTetraPointIterator;
	typedef decltype(boost::make_transform_iterator(tetinfo.cbegin(),GetVTKPointIndices)) VTKTetraCellIterator;

	pair<VTKTetraPointIterator,VTKTetraPointIterator> getVTKTetraPoints() const {
		return make_pair(P.cbegin(),P.cend());
	}

	const vector<Tetra>& getVTKTetraCells() const { return tetinfo; }
};
