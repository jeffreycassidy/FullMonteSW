/*
 * Directed.hpp
 *
 *  Created on: Jan 27, 2016
 *      Author: jcassidy
 */

#ifndef KERNELS_SOFTWARE_DIRECTED_HPP_
#define KERNELS_SOFTWARE_DIRECTED_HPP_

#include "../Packet.hpp"

namespace Emitter
{

/** Saves and returns a specific direction with no variability */

class Directed
{
public:
	Directed(){};

	/// Constructs from a provided direction
	Directed(const PacketDirection& dir) :
		m_dir(dir){}

	/// Returns the constant direction
	template<class RNG>PacketDirection direction(RNG& rng) const
	{
		return m_dir;
	}

private:
	PacketDirection		m_dir;				///< The direction object
};

};


#endif /* KERNELS_SOFTWARE_DIRECTED_HPP_ */
