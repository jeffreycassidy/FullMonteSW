/*
 * SFMT_URNG.hpp
 *
 *  Created on: Feb 12, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_BLOCKRNGADAPTOR_HPP_
#define KERNELS_SOFTWARE_BLOCKRNGADAPTOR_HPP_

#include <boost/align/align.hpp>
#include <boost/align/is_aligned.hpp>

#include <stdexcept>

namespace URNG
{
	template<typename RNG>void seed(RNG& rng,unsigned s);
	template<template RNG>void fill(RNG& rng,T* dst,std::size_t N);
};

/** BlockRNG - block random-number generation
 *
 * NOTE: Call to getBlock may invalidate any previous pointers returned by getBlock!
 * Happens because we make every effort to avoid copying
 */

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize>class BlockRNGAdaptor
{
public:
	typedef T 							result_type;								///< The type returned
	static constexpr std::size_t 		genBlockSize = GenBlockSize;				///< Number of "T" elements per block
	static constexpr std::size_t		genBlockByteSize = GenBlockSize*sizeof(T);	///< Bytes per generated block

	static constexpr std::size_t		returnBlockSize = ReturnBlockSize;
	static constexpr std::size_t		returnBlockByteSize = ReturnBlockSize*sizeof(T);	///< Number of bytes per return block

	BlockRNGAdaptor();

	void seed(unsigned);

	/// Get the next block of random numbers - may invalidate (overwrite data contained by) pointers returned by previous calls
	const T* getBlock();

private:
	T			m_buffer[GenBlockSize];		///< Aligned storage for (BlockSize) elements of (T)
	std::size_t	m_pos=0;					///< Current position within the buffer
	RNG			m_rng;						///< The random-number generator
};

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize>BlockRNGAdaptor<RNG,T,GenBlockSize,ReturnBlockSize>::BlockRNGAdaptor()
		: m_rng(),
		  m_pos(0)
{
	if (!boost::is_aligned(m_buffer,returnBlockByteSize))
		throw std::bad_alloc();
}

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize>void BlockRNGAdaptor<RNG,T,GenBlockSize,ReturnBlockSize>::seed(unsigned s)
{
	URNG::seed(m_rng,s);
}

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize>const T* BlockRNGAdaptor<RNG,T,GenBlockSize,ReturnBlockSize>::getBlock()
{
	if (m_pos == 0)
		URNG::fill(m_rng,m_buffer,GenBlockSize);

	const T* p = m_buffer+m_pos;
	m_pos = (m_pos + ReturnBlockSize) % GenBlockSize;
	return p;
}

#endif /* KERNELS_SOFTWARE_BLOCKRNGADAPTOR_HPP_ */
