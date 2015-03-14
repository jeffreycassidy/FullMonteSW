#pragma once
#include <inttypes.h>
#include <map>

#include <FullMonte/Geometry/newgeom.hpp>
#include <FullMonte/Geometry/TetraMesh.hpp>

#include <FullMonte/Geometry/Material.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/iterator/transform_iterator.hpp>

using namespace std;

template<class T>class FluenceMap;

template<class T>class PointIDLookup {
    protected:
    const TetraMesh& mesh;
    public:
    static const int dtype;

    typedef pair<unsigned,double> type;
    typedef pair<T,double> result_type;

    PointIDLookup(const PointIDLookup& lu_) : mesh(lu_.mesh){}
    PointIDLookup(const TetraMesh* mesh_)   : mesh(*mesh_){ if (!mesh_) throw typename FluenceMap<T>::InvalidMesh(); }

    // looks up the IDps (FaceByPointID/TetraByPointID) type, returning an unsigned ID
    unsigned operator()(const T&) const;

    pair<T,double> operator()(const pair<unsigned,double>& p) const { return make_pair(operator()(p.first),p.second); }
    T operator()(unsigned) const;
};

template<class T>class AreaMult : public PointIDLookup<T> {
/*    using PointIDLookup<T>::mesh;
    double getArea(unsigned) const;
    public:
    typedef pair<unsigned,double> result_type;
    typedef pair<unsigned,double> type;
    AreaMult(const AreaMult& am_)    : PointIDLookup<T>(&am_.mesh){}
    AreaMult(const TetraMesh* mesh_) : PointIDLookup<T>(mesh_){}
    pair<unsigned,double> operator()(const pair<unsigned,double>& p) const
        { return make_pair(p.first,p.second*getArea(p.first)); }*/
};

template<>class AreaMult<TetraByPointID> : public PointIDLookup<TetraByPointID> {
    using PointIDLookup<TetraByPointID>::mesh;
//    const vector<Material>& mat;

    public:
    typedef pair<unsigned,double> result_type;
    typedef pair<unsigned,double> type;

//    AreaMult(const AreaMult& am_) : PointIDLookup<TetraByPointID>(am_),mat(am_.mat){}
    AreaMult(const AreaMult& am_) : PointIDLookup<TetraByPointID>(am_){}
//    AreaMult(const TetraMesh* mesh_,const vector<Material>& mat_) : PointIDLookup<TetraByPointID>(mesh_),mat(mat_){}
    AreaMult(const TetraMesh* mesh_) : PointIDLookup<TetraByPointID>(mesh_){}
    pair<unsigned,double> operator()(const pair<unsigned,double>& p) const
//        { return make_pair(p.first,p.second*mesh.getTetraVolume(p.first)/mat[mesh.getMaterial(p.first)].getMuA()); }
        { return make_pair(p.first,p.second*mesh.getTetraVolume(p.first)); }

};

template<>class AreaMult<FaceByPointID> : public PointIDLookup<FaceByPointID> {
    using PointIDLookup<FaceByPointID>::mesh;

    public:
    typedef pair<unsigned,double> result_type;
    typedef pair<unsigned,double> type;

    AreaMult(const AreaMult& am_) : PointIDLookup<FaceByPointID>(am_){}
    AreaMult(const TetraMesh* mesh_) : PointIDLookup<FaceByPointID>(mesh_){}
    pair<unsigned,double> operator()(const pair<unsigned,double>& p) const
        { return make_pair(p.first,p.second*mesh.getFaceArea(p.first)); }
};

//template<class A,class B>const B& getFirst(const pair<A,B>&  p){ return p.first;  }
template<class A,class B>const B& getSecond(const pair<A,B>& p){ return p.second; }

class FluenceMapBase {
    protected:
    // Maps from the unsigned Face ID (IDf) to a double-precision fluence value
    map<unsigned,double> F;

    unsigned long long packets;

    // provides a mapping from FaceByPointID (sorted) to an unsigned faceID
    const TetraMesh* mesh;

    public:

    virtual ~FluenceMapBase(){}
    class InvalidMesh {};
    class InvalidBlobSize {};

    FluenceMapBase(const string& fn_,unsigned long long packets_=0) : mesh(NULL){ loadASCII(fn_,packets); }
    FluenceMapBase(const TetraMesh* mesh_=NULL,unsigned long long packets_=0) : packets(packets_),mesh(mesh_){}
    FluenceMapBase(const vector<double>& d,unsigned long long packets_=0);
    FluenceMapBase(const map<unsigned,double>& F_,const TetraMesh* mesh_,unsigned long long packets_=0) : F(F_),packets(packets_),mesh(mesh_){}

    void loadASCII(const string& fn,unsigned long long packets_=0);

    unsigned getNNZ() const { return F.size(); }

    void clear(){ F.clear(); }
    unsigned size(){ return F.size(); }
    void setMesh(const TetraMesh* mesh_){ mesh=mesh_; }

    // dereferences to a pair<unsigned,double>
    typedef map<unsigned,double>::const_iterator    const_iterator;
    typedef map<unsigned,double>::iterator          iterator;

    iterator insert(iterator it,pair<unsigned,double> p){ return F.insert(it,p); }

    iterator begin() { return F.begin(); }
    iterator   end() { return F.end();   }

    const_iterator begin() const { return F.begin(); }
    const_iterator end()   const { return F.end(); }

    double& operator[](unsigned IDf)    { return F[IDf]; }
    double& operator[](int IDf)         { return F[unsigned(abs(IDf))]; }

    // iterator returning only the fluence values, dereferences to a double
    typedef boost::transform_iterator<const double&(*)(const pair<unsigned,double>&),const_iterator> const_value_iterator;
    const_value_iterator valuesBegin() const { return boost::make_transform_iterator(F.begin(),&getSecond<unsigned,double>); }
    const_value_iterator valuesEnd()   const { return boost::make_transform_iterator(F.end(),  &getSecond<unsigned,double>); }

    // Add/Subtract another fluence map
    FluenceMapBase& operator-=(const FluenceMapBase& m);
    FluenceMapBase& operator+=(const FluenceMapBase& m);
    FluenceMapBase& operator/=(const FluenceMapBase& m);

    FluenceMapBase& operator*=(double);

    void absdiff();

    void writeASCII(string fn);

    unsigned long long getPacketCount() const { return packets; }

    // convert to vector
    vector<double> toVector(unsigned N) const;

    // Serialization to/from binary format
    string    toBinary()              const;
    bool    fromBinary(const string&,unsigned long N=-1);
    bool	fromBinary(istream& is,unsigned long N=-1);
};

template<class T>class FluenceMap : public FluenceMapBase {
    const vector<Material>* mat;
    public:
    static const int dtype;

    virtual void bunga(){}; // need this so we can dynamic_cast

    using FluenceMapBase::operator[];
	FluenceMap(const TetraMesh& mesh_,const string& b_,unsigned long long packets_=0) : FluenceMapBase(&mesh_),mat(NULL)
        { fromBinary(b_,packets_); }

    FluenceMap(const TetraMesh* mesh_) : FluenceMapBase(mesh_), mat(NULL){}

    typedef T PointIDType;
    typedef PointIDLookup<T> PointIDLookupType;

    typedef boost::transform_iterator<AreaMult<T>,const_iterator> const_energy_iterator;

    // returns a pair<PointIDType,double> with the fluence values
    typedef boost::transform_iterator<PointIDLookupType,const_iterator> const_IDp_fluence_iterator;

    const_IDp_fluence_iterator fluenceByIDpBegin() const
        { return boost::make_transform_iterator(F.begin(),PointIDLookupType(mesh)); }
    const_IDp_fluence_iterator   fluenceByIDpEnd() const
        { return boost::make_transform_iterator(F.end(),PointIDLookupType(mesh)); }
    
    // returns an iterator for the energy (fluence*area or fluence*volume)
    const_energy_iterator energyBegin() const
        { return boost::make_transform_iterator(F.begin(),AreaMult<PointIDType>(mesh)); }
    const_energy_iterator energyEnd()   const
        { return boost::make_transform_iterator(F.end(),AreaMult<PointIDType>(mesh)); }

    // read/write in TIMOS-compatible form
    void    writeTIMOS(const TetraMesh&,string fn);

    // finds the total energy
    double getTotalEnergy() const {
        double sum=0.0;
        for(const_energy_iterator it=energyBegin(); it != energyEnd(); ++it)
            sum += it->second;
        return sum;
    };

    // Element access
    double& operator[](PointIDType f)   { return F[PointIDLookupType(mesh)(f)]; }
};

typedef FluenceMap<FaceByPointID>  SurfaceFluenceMap;
typedef FluenceMap<TetraByPointID> VolumeFluenceMap;
