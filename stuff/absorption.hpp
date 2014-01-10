#include <stdio.h>
#include <string>

using namespace std;

// Enable additional features

// Use PTHREADS to do file output (default ON)
#ifndef ABS_TRACE_NO_PTHREAD
#define ABS_TRACE_PTHREAD
#endif

#ifdef ABS_TRACE_PTHREAD
#include <pthread.h>
#include <semaphore.h>
#endif

// Debug options
#ifndef ABSORPTION_HPP_INCLUDED
#define ABSORPTION_HPP_INCLUDED

// Enable bounds checking for absorption events
#define ABS_CHECK_BOUNDS

// Define locality enhancements on absorption grid
// Bit fields of memory address are organized as:
// (Upper Nz-Lz of z) (Upper Ny-Ly of y) (Upper Nx-Lx of x) (Lower Lz of z) (Lower Ly of y) (Lower Lx of x)
#ifndef ABS_REORDER_LX
    #define ABS_REORDER_LX 2
#endif
#ifndef ABS_REORDER_LY
    #define ABS_REORDER_LY 2
#endif
#ifndef ABS_REORDER_LZ
    #define ABS_REORDER_LZ 2
#endif

// Intel Nehalem/Westmere: 32-byte cache line size (256b / 4 64b ints)
// Intel Core i7: 64-byte cache line (512b / 8 64b ints)


typedef struct Point3i_t { int x,y,z; } Point3i;

template<class T>class Absorption {
    protected:
    int Np;
    T*  s;
    public:
    int addrMap(Point3i p){ return addrMap(p.x,p.y,p.z); }
    virtual int addrMap(int,int,int) const =0;
    virtual Point3i addrInv(int) const =0;

    public:
    Absorption(int Np_) : Np(Np_),s(new T[Np_]){}
    ~Absorption() { delete s; }
    void record(int,int,int,T);
    T query(int,int,int);

//    virtual T& operator()(int x,int y,int z) { return s[addrMap(x,y,z)]; }
//    virtual const T& operator()(int x,int y,int z) const { return s[addrInv(x,y,z)]; };
};

template<class T>class AbsorptionCompact : public Absorption<T> {
    using Absorption<T>::s;
    using Absorption<T>::Np;
    int Nx,Ny,Nz;
    protected:
    public:
    virtual int addrMap(int,int,int) const;
    virtual Point3i addrInv(int) const;
    AbsorptionCompact(int Nx_,int Ny_,int Nz_) : Nx(Nx_),Ny(Ny_),Nz(Nz_),
        Absorption<T>(Nx*Ny*Nz){};
};

template<class T>class AbsorptionBitwise : public Absorption<T> {
    protected:
    using Absorption<T>::Np;
    int Nb_x,Nb_y,Nb_z;
    public:
    virtual int addrMap(int,int,int) const;
    virtual Point3i addrInv(int) const;
    public:
    AbsorptionBitwise(int Nb_x_,int Nb_y_,int Nb_z_) : Absorption<T>(1 << (Nb_x_+Nb_y_+Nb_z_)),
        Nb_x(Nb_x_),Nb_y(Nb_y_),Nb_z(Nb_z_) {}
};

template<class T,int Lx,int Ly,int Lz>class AbsorptionReorder : public AbsorptionBitwise<T> {
    using AbsorptionBitwise<T>::Nb_x;
    using AbsorptionBitwise<T>::Nb_y;
    using AbsorptionBitwise<T>::Nb_z;
    protected:
    using AbsorptionBitwise<T>::Np;
    public:
    AbsorptionReorder(int Nb_x_,int Nb_y_,int Nb_z_) : AbsorptionBitwise<T>(Nb_x_,Nb_y_,Nb_z_){};
    virtual int addrMap(int,int,int) const;
    virtual Point3i addrInv(int) const;
};

template<class T>class AbsorptionTrace : public AbsorptionCompact<T> {
    FILE* f;
    struct ofstruct { int x,y,z; T a; };
    struct ofstruct *buf;
    int bufsize,chunksize;                // bufsize is number of elements
    int idx,wrIdx;

#ifdef ABS_TRACE_PTHREAD
    volatile bool live;
    sem_t     writeSem,bufSem;
    pthread_t tidWriter;        // Thread ID of writer
    static void* threadEntry(void*);
    void writerThread();
#endif

    void tryWriteChunk();
    bool writeChunk(int);

    public:
    AbsorptionTrace(int Nx_,int Ny_,int Nz_,string fn,int bufsize_=4096,int writesize_=1024);
    ~AbsorptionTrace();

    void record(int,int,int,T);

    virtual int addrMap() const;
    virtual Point3i addrInv(int) const;
};

#endif
