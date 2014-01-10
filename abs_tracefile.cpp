#include <iostream>
#include <stdio.h>
#include "absorption.hpp"
#include <errno.h>

using namespace std;

template<class T>AbsorptionTrace<T>::AbsorptionTrace(int Nx_,int Ny_,int Nz_,string fn,int bufsize_,int chunksize_) :
    bufsize(bufsize_),chunksize(chunksize_),AbsorptionCompact<T>(Nx_,Ny_,Nz_),wrIdx(0),idx(0)
{
    // open file for writing
    if( f=fopen(fn.c_str(),"w") == NULL)
        cerr << "Failed to open file " << fn << " for writing" << endl;

#ifdef ABS_TRACE_PTHREAD
    // create semaphores and threads
    if (sem_init(&writeSem,0,0) != 0)
        cerr << "Failed to create write semaphore" << endl;

    if (sem_init(&bufSem,0,bufsize/chunksize-1) != 0)
        cerr << "Failed to create buffer semaphore" << endl;

    live=true;

    if (pthread_create(&tidWriter,NULL,threadEntry,this) != 0)
        cerr << "Failed to create a pthread" << endl;

    cout << "Thread created for writing with ID " << tidWriter << endl;
#endif

    buf=new struct ofstruct[bufsize];
}

template<class T>void AbsorptionTrace<T>::record(int x,int y,int z,T a)
{
    AbsorptionCompact<T>::record(x,y,z,a);

    // copy value to buffer and update the pointer
    buf[idx].x=x;
    buf[idx].y=y;
    buf[idx].z=z;
    buf[idx].a=a;
    idx = (idx+1) % bufsize;

    // we're at a buffer boundary: try to write out the previous chunk (or notify helper thread to write)
    if (idx % chunksize == 0)
        tryWriteChunk();
}

template<class T>AbsorptionTrace<T>::~AbsorptionTrace()
{
#ifdef ABS_TRACE_PTHREAD
    live=false;
    if (pthread_join(tidWriter,NULL)!=0)
        cerr << "pthread_join() failed" << endl;
#endif

    if(fclose(f)!=0)
        cerr << "Failed to close file" << endl;

    delete buf;
}

#ifdef ABS_TRACE_PTHREAD
template<class T>void* AbsorptionTrace<T>::threadEntry(void* p)
{
    ((AbsorptionTrace<T>*)p)->writerThread();
    return NULL;
}

template<class T>void AbsorptionTrace<T>::writerThread()
{
    int Nw;
    while(live || sem_trywait(&writeSem)==0)
    {
        if (sem_wait(&writeSem) != 0)
            cerr << "Error returned by sem_wait" << endl;
        writeChunk(chunksize);
    }

    // write out remainder of file
    int N=(idx+bufsize-wrIdx)%bufsize;
    cout << "Writing remaining " << N << " elements to file" << endl;
    writeChunk((idx+bufsize-wrIdx)%bufsize);
    pthread_exit(NULL);
}
#endif

template<class T>void AbsorptionTrace<T>::tryWriteChunk()
{
#ifdef ABS_TRACE_PTHREAD
        // post to the writer to let it know another chunk needs to be written
        if (sem_post(&writeSem) != 0)
            cerr << "Error returned by sem_post" << endl;

        // check if buffer has free space
        int status = sem_trywait(&bufSem);
        if (status==0){
        }
        else if (errno==EAGAIN)
        {
            cerr << "Buffer-update thread has blocked waiting for writer to catch up" << endl;
            if (sem_wait(&bufSem)==0)
                cerr << "Resumed" << endl;
            else
                cerr << "sem_wait returned an error" << endl;
        }
        else if (status < 0){
            cerr << "Error returned by sem_trywait" << endl;
        }
#else
        // if we're not threaded, just do a blocking write
        writeChunk(chunksize);
#endif
}

template<class T>bool AbsorptionTrace<T>::writeChunk(int wsize)
{
    int Nw;
    Nw = fwrite(&buf[wrIdx],sizeof(ofstruct),wsize,f);

    if (Nw < 0)
        cerr << "fwrite returned an error" << endl;
    else if (Nw != wsize)
        cerr << "Failed to write entire chunk" << endl;
    wrIdx += Nw;

    return(Nw==wsize);
}

