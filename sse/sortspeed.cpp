#include <algorithm>
#include <iostream>
#include <boost/tuple/tuple.hpp>
#include <boost/timer/timer.hpp>
#include <boost/tuple/tuple_comparison.hpp>

using namespace std;

int main(int argc,char **argv)
{
    unsigned N=300000;
    boost::tuple<unsigned,unsigned,unsigned,unsigned> *p = new boost::tuple<unsigned,unsigned,unsigned,unsigned>[N];

    for(unsigned i=0;i<N;++i)
        p[i] = boost::make_tuple(rand(),rand(),rand(),rand());

    {
        boost::timer::auto_cpu_timer t;
        sort(p,p+N);
    }

    for(unsigned i=0;i<N-1;++i)
        if (p[i+1] < p[i])
            cerr << "OOPS!" << endl;
}


