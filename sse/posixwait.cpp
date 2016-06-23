#include <pthread.h>
#include <iomanip>

#include <boost/timer/timer.hpp>

// investigates timing effects of pthread locking

// conclusions:
//  time waiting for pthread_mutex_lock NOT charged
//  time spent sleep() or usleep() NOT charged

using namespace std;

void* thread_wait_function(void* p);

int main(int argc,char **argv)
{
    pthread_t thread;
    pthread_mutex_t mutex;
    boost::timer::auto_cpu_timer t;

    pthread_mutex_init(&mutex,NULL);

    pthread_create(&thread,NULL,thread_wait_function,&mutex);

    usleep(100000);
    {
        cout << "Time waiting for mutex lock: " << flush;
        boost::timer::auto_cpu_timer t;
        while(pthread_mutex_trylock(&mutex)){}
    }

    pthread_join(thread,NULL);
    pthread_mutex_destroy(&mutex);
}

void* thread_wait_function(void* p)
{
    pthread_mutex_t* mutex=(pthread_mutex_t*)p;
    cout << "Sleeping 2 seconds" << endl;
    pthread_mutex_lock(mutex);
    sleep(2);
    pthread_mutex_unlock(mutex);
    cout << "Waking up" << endl;
}
