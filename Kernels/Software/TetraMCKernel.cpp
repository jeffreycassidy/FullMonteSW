/*
 * TetraMCKernel.cpp
 *
 *  Created on: Jun 9, 2016
 *      Author: jcassidy
 */

#include "Emitters/TetraMeshEmitterFactory.hpp"
#include "RNG_SFMT_AVX.hpp"

#include "Emitters/TetraMeshEmitterFactory.cpp"

template class Emitter::TetraEmitterFactory<RNG_SFMT_AVX>;
