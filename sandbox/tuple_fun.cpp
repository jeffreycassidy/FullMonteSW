#include <iostream>
#include <iomanip>
#include <boost/tuple/tuple.hpp>

using namespace boost::tuples;
using namespace std;

int main(int argc,char **argv)
{

    unsigned a,b,c;

    tie(a,b,c) = make_tuple(1,2,3);

    cout << "a=" << a << " b=" << b << " c=" << c << endl;
}
