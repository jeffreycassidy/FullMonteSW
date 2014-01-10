#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <inttypes.h>

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>

#include "timebench.hpp"

using namespace std;

void cacheWipe(unsigned N,bool print)
{
    if(print)
        cout << "Wiping cache - " << flush;
    uint8_t* p = new uint8_t[N];
    for(unsigned i=0;i<N;++i)
        *p=i;
    if(print)
        cout << "done" << endl;
    delete[] p;
}
