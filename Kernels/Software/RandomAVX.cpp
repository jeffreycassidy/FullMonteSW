#include "RandomAVX.hpp"
#include <FullMonte/SFMT/SFMT.h>

void RNG_SFMT_AVX::refill()
{
    nextRand = randBuf;
    sfmt_fill_array32(&sfmt,(uint32_t*)randBuf,Nbuf);
}
