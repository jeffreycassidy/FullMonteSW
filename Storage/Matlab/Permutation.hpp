/*
 * Permutation.hpp
 *
 *  Created on: Jul 28, 2015
 *      Author: jcassidy
 */

#ifndef STORAGE_MATLAB_PERMUTATION_HPP_
#define STORAGE_MATLAB_PERMUTATION_HPP_

#include <vector>

#include <boost/range.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <iostream>


/** Holds an injective (one-to-one) mapping between two sets using integer indices.
 *
 * Notation
 * 	s[j]		The permuted subset
 * 	x[i]		The original sequence
 *
 * p_		The permutation indices such that s[j] = x[p[j]], j=1..Nsub
 * 				To construct sequence s[j], p[j] gives the indices to pick/says where element j came from
 *
 * q_ 		Inverse permutation indices such that j = q[i] s.t. s[q[i]] = x[i] if it exists, else q[i]=-1 ,i=1..N
 * 				For each element of original sequence x[i], q[i] says where it goes
 *
 */

template<class Index=unsigned>class Permutation
{
public:
	// construct empty
	Permutation(){}

	// construct by forwarding
	Permutation(std::vector<Index>&& p,std::vector<Index>&& q,bool doCheck=false) : p_(std::move(p)),q_(std::move(q))
	{
		if (doCheck && !check())
			throw std::logic_error("Invalid arguments supplied to Permutation(&& p,&& q)");
	}

	Permutation(Permutation<Index>&& P) : p_(std::move(P.p_)),q_(std::move(P.q_)){}

	Permutation& operator=(Permutation&&) = default;
	Permutation& operator=(const Permutation&) = default;

	~Permutation(){}

	// Build a permutation from the inverse map
	static Permutation buildFromInverse(std::vector<Index>&& q);

	// Build a permutation from the forward map
	static Permutation buildFromForward(std::size_t N,std::vector<Index>&& p);
	static Permutation buildFromForward(std::size_t N,const std::vector<Index>& p){ return buildFromForward(N,std::vector<unsigned>(p)); }

	// Build a subset of all elements satisfying a predicate
	template<class BoolRange>static Permutation<Index> buildStableFromBoolRange(const BoolRange& R,const std::size_t N=-1U);

	// Accessors for the permutation
	const std::vector<Index>& forward() const { return p_; }
	const std::vector<Index>& inverse() const { return q_; }

	// very paranoid (slow) internal consistency check, returns true if OK
	bool check() const;

	// check that permutation vector p_ is strictly monotonic
	bool monotonic() const;

	void clear()
	{
		p_.clear();
		q_.clear();
	}

	bool empty() const { return p_.empty(); }

private:
	std::vector<Index> p_;
	std::vector<Index> q_;
};

template<class Index>Permutation<Index> Permutation<Index>::buildFromInverse(std::vector<Index>&& q)
{
	Permutation P(q.size());
	P.p_.clear();

	P.q_ = std::forward(q);

	// q[j] = i   <=>    s[j] = x[i]

	for(const auto i : P.q_
			| boost::adaptors::indexed(0U)
			| boost::adaptors::filtered([](const boost::range::index_value<Index,ptrdiff_t> q){ return q.value() != -1U; }))
	{
		if (i.value() >= P.p_.size())
			P.p_.resize(i.value()+1,-1U);
		P.p_[i.value()] = i.index();
	}
}

template<class Index>Permutation<Index> Permutation<Index>::buildFromForward(const std::size_t N,std::vector<Index>&& p)
{
	std::vector<Index> q(N,-1U);

	// invert the map
	// s[j] = x[p[j]] 	<=> s[q[i]] = s[i]

	for(const auto i : p | boost::adaptors::indexed(0U))
		q[i.value()] = i.index();

	return Permutation(std::move(p),std::move(q));
}


template<class Index>template<class BoolRange>Permutation<Index> Permutation<Index>::buildStableFromBoolRange(const BoolRange& R,const std::size_t N)
{
	std::vector<Index> p,q;

	// could also traverse once and count size/# true values simultaneously

	if (N != -1U)
		p.reserve(N);

	for(const auto x : R | boost::adaptors::indexed(0U))
	{
		if (x.value())
		{
			q.push_back(p.size());
			p.push_back(x.index());
		}
		else
			q.push_back(-1U);
	}

	return Permutation<Index>(std::move(p),std::move(q));
}

template<class Index>bool Permutation<Index>::check() const
{
	std::vector<unsigned> pRefs(q_.size(),0),qRefs(p_.size(),0);

	// each p should be unique and non-null (!= -1U)
	for(const auto j : p_ | boost::adaptors::indexed(0U))
	{
		if (j.value() == -1U)
		{
			std::cout << "Null element" << std::endl;
			return false;				// invalid bijection: null element
		}
		else if (j.value() >= q_.size())
		{
			std::cout << "Forward map out of range" << std::endl;
			return false;				// invalid bijection: out of range index in forward map p_
		}
		else if (pRefs[j.value()] != 0)
		{
			std::cout << "Repeated element" << std::endl;
			return false;				// invalid bijection: repeat element
		}

		if (q_[j.value()] != j.index())
		{
			std::cout << "Incorrect inverse" << std::endl;
			return false;				// invalid bijection: violates requirement q[p[j]] == j
		}
	}

	for(const auto q : q_ | boost::adaptors::indexed(0U))
	{
		if (q.value() == -1U)
		{
			if (pRefs[q.index()] != 0)
			{
				std::cout << "Null q[i] but referenced" << std::endl;
				return false;				// invalid inverse: q[i] null but some p[j] references it
			}
		}
		else if (q.value() >= p_.size())
		{
			std::cout << "Inverse out of range" << std::endl;
			return false;				// invalid inverse: out-of-range index in inverse map q_
		}
		else
		{
			qRefs[q.value()]++;
			if (qRefs[q.value()] == 0)				// broken data structure: q[i] not null but no p[j] referencing it
			{
				std::cout << "Non-null q[i] but no matching reference" << std::endl;
				return false;
			}
			else if (qRefs[q.value()] > 1)			// not an injection (violates one-to-one)
			{
				std::cout << "Non-unique inverse reference" << std::endl;
				return false;
			}
		}
	}
	return true;
}

template<class Index>bool Permutation<Index>::monotonic() const
{
	for(unsigned j=1;j<p_.size();++j)
		if (p_[j-1] >= p_[j])
			return false;
	return true;
}


#endif /* STORAGE_MATLAB_PERMUTATION_HPP_ */
