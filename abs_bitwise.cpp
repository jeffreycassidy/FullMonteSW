#include <iostream>
#include "absorption.hpp"

using namespace std;

template<class T>int AbsorptionBitwise<T>::addrMap(int x,int y,int z) const
{
    int o = (((z << Nb_y) | y) << Nb_x) | x;
#ifdef ABS_CHECK_BOUNDS
    if (x < 0 || y < 0 || z < 0)
        cerr << "Absorption coordinate negative" << endl;
    if (x >= (1 << Nb_x) || y >= (1 << Nb_y) || z >= (1 << Nb_z))
        cerr << "Absorption coordinate out of range" << endl;
    if (o >= 1<<(Nb_x+Nb_y+Nb_z))
        cerr << "Absorption index out of range" << endl;
#endif
    return o;
}

template<class T>Point3i AbsorptionBitwise<T>::addrInv(int o) const
{
    Point3i p;
#ifdef ABS_CHECK_BOUNDS
    if (o < 0 || o >= Np)
        cerr << "Absorption index out of range" << endl;
#endif
    p.x = o                  & ((1 << Nb_x)-1);
    p.y = (o >> Nb_x)        & ((1 << Nb_y)-1);
    p.z = (o >> (Nb_y+Nb_x)) & ((1 << Nb_z)-1);
    return p;
}
