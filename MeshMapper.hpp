#include <array>
#include <vector>
#include <iostream>

#include <initializer_list>

using namespace std;

#include "sse.hpp"
#include "newgeom.hpp"

#include <boost/tuple/tuple.hpp>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>

#ifdef USE_DUMMY

class TetraMeshDummy {

    public:
    vector<array<double,3>> P;
    vector<FaceByPointID_> F;
    vector<TetraByPointID_> T_p;

    TetraMeshDummy(unsigned n_=0) : P(n_){}

    typedef vector<array<double,3> >::const_iterator point_const_iterator;
    typedef vector<FaceByPointID_>::const_iterator face_id_const_iterator;
    typedef vector<TetraByPointID_>::const_iterator tetra_const_iterator;

    unsigned getNp() const { return P.size()-1; }
    unsigned getNf() const { return F.size()-1; }
    unsigned getNt() const { return T_p.size()-1; }

    point_const_iterator pointBegin() const { return P.begin()+1; }
    point_const_iterator pointEnd()   const { return P.end(); }

    face_id_const_iterator faceIDBegin() const { return F.begin()+1; }
    face_id_const_iterator faceIDEnd() const { return F.end(); }

    tetra_const_iterator tetraIDBegin() const { return T_p.begin()+1; }
    tetra_const_iterator tetraIDEnd()   const { return T_p.end();     }
};
typedef array<unsigned long,3> FaceByPointID_;
typedef array<unsigned long,4> TetraByPointID_;

typedef TetraMeshDummy TetraMesh;

#else

typedef FaceByPointID FaceByPointID_;
typedef TetraByPointID TetraByPointID_;

#endif

template<class Payload>class LookupNonZero {
    const vector<unsigned long>& lut;
    public:
    LookupNonZero(const vector<unsigned long>& lut_) : lut(lut_){}
    LookupNonZero(const LookupNonZero& l_) = default;
    LookupNonZero(LookupNonZero&& l_) = default;

    // Lookup of a tuple<unsigned long,Payload...>
    bool operator()(const boost::tuple<unsigned long,Payload>& t) const { return lut[t.get<0>()]; }
};

template<class Idx,class Item>class Lookup {
    const vector<Item>& lut;
    public:

    typedef Idx type;
    typedef Item result_type;

    Lookup(const vector<result_type>& lut_) : lut(lut_){}
    Lookup(const Lookup& l_) = default;
    Lookup(Lookup&& l_) = default;

    result_type operator()(type i) const { return lut[i]; }
};



/** Maps a boost::tuple<unsigned long,array> => 
 * Intended for use with Face/TetraByPointID
 * For each element of the input array, maps it through the lut given in the constructor
 * ie for all i, y[i] = lut[x[i]]
 */

template<class IEl,class OEl,unsigned long N>class Lookup<boost::tuple<unsigned long,array<IEl,N> >,boost::tuple<unsigned long,array<OEl,N> > > {
    const vector<OEl>& lut;
    public:

    typedef boost::tuple<unsigned long,array<IEl,N> > type;
    typedef boost::tuple<unsigned long,array<OEl,N> > result_type;

    Lookup(const vector<OEl>& lut_) : lut(lut_){}
    Lookup(const Lookup& l_) = default;

    result_type operator()(const type& i) const {
        result_type o=i;
        transform(i.get<1>().begin(),i.get<1>().end(),o.get<1>().begin(),Lookup<IEl,OEl>(lut));
        return o;
    }
};

// typedefs to do face & tetra remapping
typedef Lookup<boost::tuple<unsigned long,FaceByPointID_>,boost::tuple<unsigned long,FaceByPointID_> > FaceRemap;
typedef Lookup<boost::tuple<unsigned long,TetraByPointID_>,boost::tuple<unsigned long,TetraByPointID_> > TetraRemap;

//template<class Idx,class Item>typedef boost::zip_iterator<boost::tuple<boost::counting_iterator<Idx>,typename vector<Item>::const_iterator> > const_indexed_iterator;

/*template<class Idx,class Item>boost::zip_iterator<boost::tuple<boost::counting_iterator<Idx>,typename vector<Item>::const_iterator> >
    make_indexed_iterator(typename vector<Item>::const_iterator it,Idx i0_=0)
{
    return make_zip_iterator(boost::make_tuple(boost::counting_iterator<Idx>(i0_),it));
}*/

template<class Idx,class ItemIterator>boost::zip_iterator<boost::tuple<boost::counting_iterator<Idx>,ItemIterator> >
    make_indexed_iterator(ItemIterator it,Idx i0_=0)
{
    return make_zip_iterator(boost::make_tuple(boost::counting_iterator<Idx>(i0_),it));
}


/** Provides an adaptor for vector such that the iterator returns a boost::tuple<Idx,T> containing the index and the vector element */

template<class Idx,class T>class vector_index_adaptor {
    const vector<T>& v;
    Idx offset;

    public:

    vector_index_adaptor(vector<T>& v_,Idx offset_) : v(v_),offset(offset_){}

    typedef boost::tuple<Idx,T> element;

    typedef boost::zip_iterator<boost::tuple<boost::counting_iterator<Idx>,typename vector<T>::const_iterator> > const_iterator;

    const_iterator begin() { return make_indexed_iterator(v.begin(),offset); }
    const_iterator   end() { return make_indexed_iterator(v.end(),offset+v.size()); }

};



/** Provides facilities for remapping a subset of points according to a lookup table for point IDs.
 * The lookup is provided as a vector<unsigned long>. If lut[i]=j  ==>  i -> j   (current index i, output index j)
 * Zero destination value means the points are not included in the output set
 *
 * PointMapper filters out all points which are not included in the output set (ie have zero destination value)
 */

class MeshMapping {
    
    public:

    /** Filters out vector elements whose destination index in the LUT is zero
     * Provides const_iterators which return a boost::tuple<unsigned long,T>
     */

    class PointMapper {
        TetraMesh& M;
        const vector<unsigned long>& Pmap;

        public:
        PointMapper(TetraMesh& M_,const vector<unsigned long>& Pmap_) : M(M_),Pmap(Pmap_){}

        typedef boost::zip_iterator<
            boost::tuple<
                boost::counting_iterator<unsigned long>,
                vector<array<double,3> >::const_iterator> > numbered_point_iterator;

        typedef boost::filter_iterator<
            LookupNonZero<array<double,3> >,
            numbered_point_iterator> const_iterator;

        const_iterator begin() const {
            return boost::filter_iterator<LookupNonZero<array<double,3> >,numbered_point_iterator>(
                LookupNonZero<array<double,3> >(Pmap),
                make_indexed_iterator<unsigned long,vector<array<double,3> >::const_iterator> (M.pointBegin(), 1U));
            }

        const_iterator end() const {
            return boost::filter_iterator<LookupNonZero<array<double,3> >,numbered_point_iterator>(
                LookupNonZero<array<double,3> >(Pmap),
                make_indexed_iterator<unsigned long,vector<array<double,3> >::const_iterator> (M.pointEnd(), M.getNp()+1));
        }
    };


    /** Filters out faces which map to zero and remaps the point IDs on the remaining faces.
     */
    class FaceMapper {
        TetraMesh& M;
        const vector<unsigned long>& Pmap;
        const vector<unsigned long>& Fmap;

        public:

        FaceMapper(TetraMesh& M_,const vector<unsigned long>& Pmap_,const vector<unsigned long>& Fmap_) : M(M_),Pmap(Pmap_),Fmap(Fmap_){}

        // append the index numbers
        typedef boost::zip_iterator<
            boost::tuple<
                boost::counting_iterator<unsigned long>,
                vector<FaceByPointID_>::const_iterator> > numbered_point_iterator;

        // filter out those which map to zero
        typedef boost::filter_iterator<
            LookupNonZero<array<unsigned long,3> >,
            numbered_point_iterator> filtered_iterator;

        typedef boost::transform_iterator<
            FaceRemap,
            filtered_iterator> const_iterator;

        const_iterator begin() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<array<unsigned long,3> >(Fmap),
                        make_indexed_iterator<unsigned long,vector<array<unsigned long,3> >::const_iterator> (M.faceIDBegin(), 1UL)),
                    FaceRemap(Pmap));
            }

        const_iterator end() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<array<unsigned long,3> >(Fmap),
                        make_indexed_iterator<unsigned long,vector<array<unsigned long,3> >::const_iterator> (M.faceIDEnd(), M.getNf()+1)),
                    FaceRemap(Pmap));
            }
    };
    
    /** Filters out tetras which map to zero and remaps the point IDs on the remaining faces.
     */
    class TetraMapper {
        TetraMesh& M;
        const vector<unsigned long>& Pmap;
        const vector<unsigned long>& Tmap;

        public:

        TetraMapper(TetraMesh& M_,const vector<unsigned long>& Pmap_,const vector<unsigned long>& Tmap_) : M(M_),Pmap(Pmap_),Tmap(Tmap_){}

        // append the index numbers
        typedef boost::zip_iterator<
            boost::tuple<
                boost::counting_iterator<unsigned long>,
                vector<TetraByPointID_>::const_iterator> > numbered_tetra_iterator;

        // filter out those which map to zero
        typedef boost::filter_iterator<
            LookupNonZero<TetraByPointID_>,
            numbered_tetra_iterator> filtered_iterator;

        typedef boost::transform_iterator<
            TetraRemap,
            filtered_iterator> const_iterator;

        const_iterator begin() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<TetraByPointID_>(Tmap),
                        make_indexed_iterator<unsigned long,vector<TetraByPointID_>::const_iterator> (M.tetraIDBegin(), 1UL)),
                    TetraRemap(Pmap));
        }

        const_iterator end() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<TetraByPointID_>(Tmap),
                        make_indexed_iterator<unsigned long,vector<TetraByPointID_>::const_iterator> (M.tetraIDEnd(), M.getNt()+1)),
                    TetraRemap(Pmap));
            }
    };

};

template<class T,unsigned long N>ostream& operator<<(ostream& os,const array<T,N>& a)
{
    unsigned i;
    for(i=0;i<N-1;++i)
        os << a[i] << ' ';
    return os << a[i];
}

