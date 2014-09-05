#include "SourceDescription.hpp"
#include "newgeom.hpp"
#include <limits>
#include <sstream>

string PencilBeamSourceDescription::timos_str(unsigned long long Np) const
{
    stringstream ss;
    ss << "11 " << IDt << ' ' << getOrigin() << ' ' << getDirection() << ' ' << (Np == 0 ? (unsigned long long)getPower() : Np);
    return ss.str();
}

string IsotropicPointSourceDescription::timos_str(unsigned long long Np) const
{
    stringstream ss;
    ss << "1 " << plainwhite << getOrigin() << ' ' << (Np == 0 ? (unsigned long long)getPower() : Np);
    return ss.str();
}

string VolumeSourceDescription::timos_str(unsigned long long Np) const
{
    stringstream ss;
    ss << "2 " << IDt << ' ' << (Np == 0 ? (unsigned long long) getPower() : Np);
    return ss.str();
}

string FaceSourceDescription::timos_str(unsigned long long Np) const
{
    stringstream ss;
    ss << "12 " << f[0] << ' ' << f[1] << ' ' << f[2] << ' ' << (Np == 0 ? (unsigned long long) getPower() : Np);
    return ss.str();
}


ostream& IsotropicPointSourceDescription::print(ostream& os) const
{
	return os << "Isotropic point source located " << getOrigin() << " with weight " << getPower();
}

ostream& PencilBeamSourceDescription::print(ostream& os) const
{
	os << "Pencil beam source located " << getOrigin() << " aimed ";
	os << getDirection() << " entering at IDf=" << IDf << " IDt=" << IDt << " with weight " << getPower();
	return os;
}

ostream& VolumeSourceDescription::print(ostream& os) const
{
    return os << "Volume source located in tetrahedron IDt=" << IDt << " with weight " << getPower();
}

ostream& operator<<(ostream& os,SourceDescription& src)
{
	return src.print(os);
}


ostream& FaceSourceDescription::print(ostream& os) const
{
    return os << "Face source, IDf=" << IDf << " Points " << f << " Direction " << n;
}


string SourceMultiDescription::timos_str(unsigned long long Npacket) const
{
    string str;
    for(vector<SourceDescription*>::const_iterator it=sources.begin(); it != sources.end(); ++it)
        str += (*it)->timos_str() + "\n";
    return str;
}

ostream& SourceMultiDescription::print(ostream& os) const
{
    for(vector<SourceDescription*>::const_iterator it=sources.begin(); it != sources.end(); ++it)
        (*it)->print(os) << endl;
    return os;
}
