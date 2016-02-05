#pragma once

#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/any_range.hpp>

#include <vector>
#include "SparseVector.hpp"
#include "DenseVector.hpp"

using namespace std;

/** Represents a spatial map of some property over elements in a geometry */

template<typename Value,typename Index=unsigned>class SpatialMapBase
{
public:
	virtual ~SpatialMapBase(){}

	static SpatialMapBase* newFromVector(const std::vector<Value>& v);

	virtual boost::any_range<const std::pair<Index,Value>,boost::forward_traversal_tag> 	nonzeros() const=0;
	virtual boost::any_range<const std::pair<Index,Value>,boost::forward_traversal_tag> 	dense() const=0;
	virtual boost::any_range<const Value,boost::forward_traversal_tag>						values() const=0;

	virtual unsigned											nnz() const=0;
	virtual unsigned											dim() const=0;
	virtual Value												operator[](unsigned i) const=0;

	virtual Value												sum() const=0;
};




/** Specific instance of SpatialMap using a container (DenseVector/SparseVector/other)
 */

template<class Container>class SpatialMapContainer : public SpatialMapBase<typename Container::Value,typename Container::Index>
{
public:
	template<class... Args>SpatialMapContainer(Args... args) : m_container(args...){}

	typedef typename Container::Value Value;
	typedef typename Container::Index Index;

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
};



template<typename Value,typename Index>SpatialMapBase<Value,Index>* SpatialMapBase<Value,Index>::newFromVector(const std::vector<Value>& v)
{
	unsigned nnz=0;
	for(const auto val : v)
		nnz += val != 0;

	unsigned Nbsparse = (sizeof(Value)+sizeof(Index))*nnz;
	unsigned Nbdense  = sizeof(Value)*v.size();

	SpatialMapBase<Value,Index>* m=nullptr;

	cout << "Spatial map with dim=" << v.size() << " nnz=" << nnz << "   Byte sizes: sparse=" << Nbsparse << " dense=" << Nbdense << endl;

	if (Nbsparse < Nbdense)
	{
		cout << "Creating sparse vector" << endl;
		m = new SpatialMapContainer<SparseVector<Value,Index>>(value_range_t(), v, nnz);
	}
	else
	{
		cout << "Creating dense vector" << endl;
		m = new SpatialMapContainer<DenseVector<Value,Index>>(v);
	}

	cout << "  Confirming dim=" << m->dim() << " nnz=" << m->nnz() << endl;
	return m;
}
