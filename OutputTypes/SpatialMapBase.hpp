#pragma once

#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/any_range.hpp>

#include <boost/serialization/serialization.hpp>

#include <vector>
#include "SparseVector.hpp"
#include "DenseVector.hpp"

using namespace std;

/** Represents a spatial map of some property over elements in a geometry */

template<typename Value,typename Index=unsigned>class SpatialMapBase
{
public:
	virtual ~SpatialMapBase(){}

	static SpatialMapBase* newFromVector(const std::vector<Value>& v)
		{ return newFromVector(std::vector<Value>(v)); }
	static SpatialMapBase* newFromVector(std::vector<Value>&& v);

	virtual SpatialMapBase* clone() const=0;

	virtual boost::any_range<const std::pair<Index,Value>,boost::forward_traversal_tag> 	nonzeros() const=0;
	virtual boost::any_range<const std::pair<Index,Value>,boost::forward_traversal_tag> 	dense() const=0;
	virtual boost::any_range<const Value,boost::forward_traversal_tag>						values() const=0;

	virtual unsigned											nnz() const=0;
	virtual unsigned											dim() const=0;
	virtual Value												operator[](unsigned i) const=0;

	virtual Value												sum() const=0;

private:
	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{ }
	friend class boost::serialization::access;
};




/** Specific instance of SpatialMap using a container (DenseVector/SparseVector/other)
 */

template<class Container>class SpatialMapContainer : public SpatialMapBase<typename Container::Value,typename Container::Index>
{
public:
	template<class... Args>SpatialMapContainer(Args... args) : m_container(args...){}
	SpatialMapContainer(const Container& C) : m_container(C){}
	SpatialMapContainer(Container&& C) : m_container(std::move(C)){}


	typedef typename Container::Value Value;
	typedef typename Container::Index Index;

	virtual SpatialMapBase<Value,Index>* clone() const override { return new SpatialMapContainer(m_container); }

	virtual boost::any_range<const std::pair<Index,Value>,boost::forward_traversal_tag> 	nonzeros() const final override
		{ return m_container.nonzeros(); 	}

	virtual boost::any_range<const std::pair<Index,Value>,boost::forward_traversal_tag> 	dense() const final override
		{ return m_container.dense(); 		}

	virtual boost::any_range<const Value,boost::forward_traversal_tag>						values() const final override
		{ return m_container.values(); 		}

	virtual unsigned														nnz() const final override
		{ return m_container.nnz();			}

	virtual unsigned														dim() const final override
		{ return m_container.dim(); 		}

	virtual Value															operator[](unsigned i) const final override
		{ return m_container[i];			}

	virtual Value															sum() const final override
		{ return m_container.sum();			}

protected:
	Container m_container;

	typedef SpatialMapBase<typename Container::Value,typename Container::Index> Base;

	template<class Archive>void serialize(Archive& ar,const unsigned ver)
		{
		 	 boost::serialization::void_cast_register(
						static_cast<SpatialMapContainer*>(nullptr),
						static_cast<Base*>(nullptr));

		 	 // causes breakage (crash) for unknown reason
		 	 //ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Base);

			ar & boost::serialization::make_nvp("values",m_container);
		}
	friend class boost::serialization::access;
};


template<typename Value,typename Index>SpatialMapBase<Value,Index>* SpatialMapBase<Value,Index>::newFromVector(std::vector<Value>&& v)
{
	unsigned nnz=0;
	for(const auto val : v)
		nnz += nonzero(val);

	unsigned Nbsparse = (sizeof(Value)+sizeof(Index))*nnz;
	unsigned Nbdense  = sizeof(Value)*v.size();

	SpatialMapBase<Value,Index>* m=nullptr;

	if (Nbsparse < Nbdense)
	{
		m = new SpatialMapContainer<SparseVector<Value,Index>>(value_range_t(), v, nnz);
		v.clear();
	}
	else
		m = new SpatialMapContainer<DenseVector<Value,Index>>(std::move(v));

	return m;
}


typedef SpatialMapBase<float,unsigned> SpatialMapBaseFU;
typedef SpatialMapContainer<SparseVector<float,unsigned>> SparseVectorFU;
typedef SpatialMapContainer<DenseVector<float,unsigned>> DenseVectorFU;
