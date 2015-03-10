#pragma once

#include "TetraMeshBase.hpp"

#include "newgeom.hpp"

using namespace std;

typedef struct { 
        __m128 Pe;          // 4x16B = 64 B
        __m128 distance;
        int IDfe;           // 4B
        unsigned IDte;      // 4B
        int idx;            // 4B
        bool hit;           // 1B
    } StepResult;

struct Tetra {
    __m128 nx,ny,nz,C;      // 4 x 16B = 64B
    TetraByFaceID   IDfs;   // 4 x 4B = 16 B
    unsigned adjTetras[4];  // 4 x 4B = 16 B
    unsigned matID;         // 4 B

    bool pointWithin(__m128) const;

    StepResult getIntersection(__m128,__m128,__m128 s) const;
    
} __attribute__ ((aligned(64)));

