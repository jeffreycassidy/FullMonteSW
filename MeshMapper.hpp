#ifndef MESHMAPPER_INCLUDED
#define MESHMAPPER_INCLUDED
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


/** TODO: See if can use Boost permutation iterator for these purposes
 * It takes two sequences i_m,j_n and iterates over i[j_k]
 * We currently do the reverse, where the permutation index j_k says where i_k goes to
 */


/** Maps a boost::tuple<unsigned long,array> => 
 * Intended for use with Face/TetraByPointID
 * For each element of the input array, maps it through the lut given in the constructor
 * ie for all i, y[i] = lut[x[i]]
 */

/*template<class IEl,class OEl,unsigned long N>class Lookup<boost::tuple<unsigned long,array<IEl,N> >,boost::tuple<unsigned long,array<OEl,N> > > {
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
};*/

template<class T>class Lookup<boost::tuple<unsigned long,T>,boost::tuple<unsigned long,T> > {
    const vector<unsigned long>& lut;
    public:

    typedef boost::tuple<unsigned long,T> type;
    typedef boost::tuple<unsigned long,T> result_type;

    Lookup(const vector<unsigned long>& lut_) : lut(lut_){}
    Lookup(const Lookup& l_) = default;

    result_type operator()(const type& i) const {
        result_type o=i;
        transform(i.get<1>().begin(),i.get<1>().end(),o.get<1>().begin(),Lookup<unsigned long,unsigned long>(lut));
        return o;
    }
};




// typedefs to do face & tetra remapping
typedef Lookup<boost::tuple<unsigned long,FaceByPointID>,boost::tuple<unsigned long,FaceByPointID> > FaceRemap;
typedef Lookup<boost::tuple<unsigned long,TetraByPointID>,boost::tuple<unsigned long,TetraByPointID> > TetraRemap;

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

template<class Idx,class Item>Item GetPayload(const boost::tuple<Idx,Item>& i){ return get<1>(i); }

/** Provides facilities for remapping a subset of points according to a lookup table for point IDs.
 * The lookup is provided as a vector<unsigned long>. If lut[i]=j  ==>  i -> j   (current index i, output index j)
 * Zero destination value means the points are not included in the output set
 *
 * PointMapper filters out all points which are not included in the output set (ie have zero destination value)
 */

template<class T>bool nonzero(T x){ return x != 0; }

class MeshMapper {
	vector<unsigned long> Pmap,Fmap,Tmap;
	const TetraMesh& M;

	unsigned Np,Nf,Nt;
    
    public:

	unsigned getNp() const { return Np; }
	unsigned getNf() const { return Nf; }
	unsigned getNt() const { return Nt; }

	// TODO: Could save a bit of time by using forwarding/move semantics for the vectors here
	MeshMapper(const TetraMesh& M_,const vector<unsigned long>& Pmap_,const vector<unsigned long>& Fmap_,const vector<unsigned long>& Tmap_) :
		Pmap(Pmap_),Fmap(Fmap_),Tmap(Tmap_),M(M_),
		Np(count_if(Pmap_.begin(),Pmap_.end(),nonzero<unsigned>)),
		Nf(count_if(Fmap_.begin(),Fmap_.end(),nonzero<unsigned>)),
		Nt(count_if(Tmap_.begin(),Tmap_.end(),nonzero<unsigned>)),
		pm(M,Pmap),
		fm(M,Pmap,Fmap),
		tm(M,Pmap,Tmap){}

    /** Filters out vector elements whose destination index in the LUT is zero
     * Provides const_iterators which return a boost::tuple<unsigned long,T>
     */

    class PointMapper {
        const TetraMesh& M;
        const vector<unsigned long>& Pmap;

        public:
        PointMapper(const TetraMesh& M_,const vector<unsigned long>& Pmap_) : M(M_),Pmap(Pmap_){}

        typedef boost::zip_iterator<
            boost::tuple<
                boost::counting_iterator<unsigned long>,
                vector<Point<3,double>>::const_iterator> > numbered_point_iterator;

        typedef boost::filter_iterator<
            LookupNonZero<Point<3,double>>,
            numbered_point_iterator> const_iterator;

        const_iterator begin() const {
            return boost::filter_iterator<LookupNonZero<Point<3,double>>,numbered_point_iterator>(
                LookupNonZero<Point<3,double>>(Pmap),
                make_indexed_iterator<unsigned long,vector<Point<3,double>>::const_iterator> (M.pointBegin(), 1UL));
            }

        const_iterator end() const {
            return boost::filter_iterator<LookupNonZero<Point<3,double>>,numbered_point_iterator>(
                LookupNonZero<Point<3,double>>(Pmap),
                make_indexed_iterator<unsigned long,vector<Point<3,double>>::const_iterator> (M.pointEnd(), M.getNp()+1UL));
        }
    };


    /** Filters out faces which map to zero and remaps the point IDs on the remaining faces.
     */
    class FaceMapper {
    	const TetraMesh& M;
        const vector<unsigned long>& Pmap;
        const vector<unsigned long>& Fmap;

        public:

        FaceMapper(const TetraMesh& M_,const vector<unsigned long>& Pmap_,const vector<unsigned long>& Fmap_)
        	: M(M_),Pmap(Pmap_),Fmap(Fmap_){}

        // append the index numbers
        typedef boost::zip_iterator<
            boost::tuple<
                boost::counting_iterator<unsigned long>,
                vector<FaceByPointID>::const_iterator> > numbered_point_iterator;

        // filter out those which map to zero
        typedef boost::filter_iterator<
            LookupNonZero<FaceByPointID>,
            numbered_point_iterator> filtered_iterator;

        typedef boost::transform_iterator<
            FaceRemap,
            filtered_iterator> const_iterator;

        const_iterator begin() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<FaceByPointID>(Fmap),
                        make_indexed_iterator<unsigned long,vector<FaceByPointID>::const_iterator> (M.faceIDBegin(), 1UL)),
                    FaceRemap(Pmap));
            }

        const_iterator end() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<FaceByPointID>(Fmap),
                        make_indexed_iterator<unsigned long,vector<FaceByPointID>::const_iterator> (M.faceIDEnd(), M.getNf()+1UL)),
                    FaceRemap(Pmap));
            }
    };
    
    /** Filters out tetras which map to zero and remaps the point IDs on the remaining faces.
     */
    class TetraMapper {
    	const TetraMesh& M;
        const vector<unsigned long>& Pmap;
        const vector<unsigned long>& Tmap;

        public:

        TetraMapper(const TetraMesh& M_,const vector<unsigned long>& Pmap_,const vector<unsigned long>& Tmap_)
        	: M(M_),Pmap(Pmap_),Tmap(Tmap_){}

        // append the index numbers
        typedef boost::zip_iterator<
            boost::tuple<
                boost::counting_iterator<unsigned long>,
                vector<TetraByPointID>::const_iterator> > numbered_tetra_iterator;

        // filter out those which map to zero
        typedef boost::filter_iterator<
            LookupNonZero<TetraByPointID>,
            numbered_tetra_iterator> filtered_iterator;

        typedef boost::transform_iterator<
            TetraRemap,
            filtered_iterator> const_iterator;

        const_iterator begin() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<TetraByPointID>(Tmap),
                        make_indexed_iterator<unsigned long,vector<TetraByPointID>::const_iterator> (M.tetraIDBegin(), 1UL)),
                    TetraRemap(Pmap));
        }

        const_iterator end() const {
                return const_iterator(
                    filtered_iterator(
                        LookupNonZero<TetraByPointID>(Tmap),
                        make_indexed_iterator<unsigned long,vector<TetraByPointID>::const_iterator> (M.tetraIDEnd(), M.getNt()+1)),
                    TetraRemap(Pmap));
            }
    };

    private:

    PointMapper pm;
    FaceMapper  fm;
    TetraMapper tm;

    public:

    vector<unsigned long> getPointPermutation() const {
    	vector<unsigned long> perm(Np,0);
    	for(unsigned i=0,j=0; i < Pmap.size(); ++i)
    		if (Pmap[i] != 0)
    			perm[j++]=i;
    	return perm;
    }

    vector<unsigned long> getFacePermutation() const {
    	vector<unsigned long> perm(Nf,0);
    	for(unsigned i=0,j=0; i < Fmap.size(); ++i)
    		if (Fmap[i] != 0)
    			perm[j++]=i;
    	return perm;
    }

    const PointMapper& points=pm;
    const TetraMapper& tetras=tm;
    const FaceMapper&  faces=fm;
};

template<class T,unsigned long N>ostream& operator<<(ostream& os,const array<T,N>& a)
{
    unsigned i;
    for(i=0;i<N-1;++i)
        os << a[i] << ' ';
    return os << a[i];
}

#endif
