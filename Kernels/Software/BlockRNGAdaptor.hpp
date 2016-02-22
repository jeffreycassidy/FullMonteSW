/*
 * BlockRNGAdaptor.hpp
 *
 *  Created on: Feb 12, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_BLOCKRNGADAPTOR_HPP_
#define KERNELS_SOFTWARE_BLOCKRNGADAPTOR_HPP_

#include <boost/align/align.hpp>
#include <boost/align/is_aligned.hpp>

#include <stdexcept>



/** URNG is a namespace that contains free functions supporting BlockRNG. Any class passed as an RNG to BlockRNGAdaptor must
 * have template specializations for seed(rng,s) and generate(rnd,dst,N).
 */

namespace URNG
{
	template<typename RNG>void 				seed(RNG& rng,unsigned s);						///< Seed RNG with unsigned value
	template<typename RNG,typename T>void 	generate(RNG& rng,T* dst,std::size_t N);		///< Generate N elements of type T
};



/** BlockRNGAdaptor - Block random-number generation: generates and caches batches of random numbers, returning blocks on
 * request.
 *
 * NOTE: Call to getBlock may invalidate any previous pointers returned by getBlock, because efforts are made to avoid copying.
 *
 * @tparam		RNG				Random-number generator with the overrides required in namespace URNG
 * @tparam		T				The type of random numbers returned
 * @tparam		GenBlockSize	Number of random numbers generated when the RNG is invoked
 * @tparam		ReturnBlockSize	Number of random numbers returned for each call to getBlock()
 * @tparam		Align			Byte alignment of the returned random numbers
 */

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize,std::size_t Align=ReturnBlockSize*sizeof(T)>
	class alignas(ReturnBlockSize*sizeof(T)) BlockRNGAdaptor
{
public:
	typedef T 							result_type;								///< The type returned
	static constexpr std::size_t 		genBlockSize = GenBlockSize;				///< Number of "T" elements per block generated
	static constexpr std::size_t		genBlockByteSize = GenBlockSize*sizeof(T);	///< Bytes per generated block

	static constexpr std::size_t		returnBlockSize = ReturnBlockSize;					///< Number of "T" getBlock() returns
	static constexpr std::size_t		returnBlockByteSize = ReturnBlockSize*sizeof(T);	///< Number of bytes per return block

	/// Default-construct, check alignment of buffer, and seed the RNG if seed != -1U
	BlockRNGAdaptor(unsigned seed=-1U);

	/// Seed the RNG using the template specialization in URNG
	void seed(unsigned);

	/// Get the next block of random numbers - may invalidate (overwrite data contained by) pointers returned by previous calls
	const T* getBlock();

	/// Access the contained rng
	RNG& rng(){ return m_rng; }

private:
	T 			m_buffer[GenBlockSize];		///< Aligned storage for (GenBlockSize) elements of (T)
	RNG			m_rng;						///< The random-number generator
	std::size_t	m_pos=0;					///< Current position within the buffer
};

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize,std::size_t Align>BlockRNGAdaptor<RNG,T,GenBlockSize,ReturnBlockSize,Align>::BlockRNGAdaptor(unsigned rngSeed)
	: m_rng(), m_pos(0)
{
	if (!boost::alignment::is_aligned(Align,m_buffer))
		throw std::bad_alloc();

	if (rngSeed != -1U)
		seed(rngSeed);
}

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize,std::size_t Align>void BlockRNGAdaptor<RNG,T,GenBlockSize,ReturnBlockSize,Align>::seed(unsigned s)
{
	URNG::seed(m_rng,s);
}

template<class RNG,typename T,std::size_t GenBlockSize,std::size_t ReturnBlockSize,std::size_t Align>const T* BlockRNGAdaptor<RNG,T,GenBlockSize,ReturnBlockSize,Align>::getBlock()
{
	if (m_pos == 0)
		URNG::generate(m_rng,m_buffer,GenBlockSize);

	const T* p = m_buffer+m_pos;
	m_pos = (m_pos + ReturnBlockSize) % GenBlockSize;
	return p;
}

#endif /* KERNELS_SOFTWARE_BLOCKRNGADAPTOR_HPP_ */
