#include <iostream>
#include "source.hpp"

using namespace std;

class TimOSManip_tag {
    public:
    TimOSManip_tag(){};
};

class TimOSManip {
    ostream& os;
    public:
    TimOSManip() : os(cout){}
    TimOSManip(ostream& os_) : os(os_){}
    friend TimOSManip operator<<(ostream& os,const TimOSManip &p);
    friend ostream& operator<<(const TimOSManip&,const Source&);
    friend ostream& operator<<(const TimOSManip&,const IsotropicPointSource&);
};

const TimOSManip timos(cout);

TimOSManip operator<<(ostream& os,const TimOSManip& p)
{
    return TimOSManip(os);
}

// prints a source to a timosmanip
ostream& operator<<(const TimOSManip& t,const Source& s)
{
    t.os << s() << ":" << s.getPower();
    return t.os;
}

ostream& operator<<(const TimOSManip& t,const IsotropicPointSource& s)
{
    t.os << "IT's isotropic!" << endl;
    return t.os;
}

int main(int argc,char **argv)
{
    double p[3] = { 1.0,2.0,-1.0 };
    Point<3,double> p0(p);
    IsotropicPointSource s(p0,10.0);
    Source *ps = &s;

//    cout << s << endl;
    cerr << timos << *ps;
}
