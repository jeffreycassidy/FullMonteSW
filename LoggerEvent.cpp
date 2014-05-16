#include "LoggerEvent.hpp"

const LoggerEvent& LoggerEvent::operator+=(const LoggerEvent& rhs)
{
    Nlaunch     += rhs.Nlaunch;
    Nabsorb     += rhs.Nabsorb;
    Nscatter    += rhs.Nscatter;
    Nbound      += rhs.Nbound;
    Ntir        += rhs.Ntir;
    Nfresnel    += rhs.Nfresnel;
    Nrefr       += rhs.Nrefr;
    Ninterface  += rhs.Ninterface;
    Nexit       += rhs.Nexit;
    Ndie        += rhs.Ndie;
    Nwin        += rhs.Nwin;
    Nabnormal	+= rhs.Nabnormal;
    return *this;
}

ostream& operator<<(ostream& os,const EventCount& ec)
{
    os << "Launched: " << ec.Nlaunch << endl;

    os << "Boundary (same):      " << ec.Nbound << endl;
    os << "Boundary (different): " << ec.Ninterface << endl;
    os << "  TIR:     " << ec.Ntir << endl;
    os << "  Fresnel: " << ec.Nfresnel << endl;
    os << "  Refract: " << ec.Nrefr << endl;
    os << "  Balance (bound - [TIR + fresnel + refract]): " << ec.Ninterface-ec.Ntir-ec.Nfresnel-ec.Nrefr << endl;

    os << "Absorption: " << ec.Nabsorb << endl;
    os << "Scatter:    " << ec.Nscatter << endl;

    os << "Roulette results" << endl;
    os << "  Win:  " << ec.Nwin << endl;
    os << "  Lose: " << ec.Ndie << endl;

    os << "End results" << endl;
    os << "Died:    " << ec.Ndie << endl;
    os << "Exited:  " << ec.Nexit << endl;
    os << "Abnormal:" << ec.Nabnormal << endl;
    os << "Balance ([launch] - [die + exit]): " << ec.Nlaunch-ec.Ndie-ec.Nexit << endl;

    return os;
}
