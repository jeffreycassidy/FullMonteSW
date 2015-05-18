#include "SourceDescription.hpp"
#include "newgeom.hpp"
#include <limits>
#include <sstream>
//
//string PencilBeamSourceDescription::timos_str(unsigned long long Np) const
//{
//    stringstream ss;
//    ss << "11 " << IDt << ' ' << getOrigin() << ' ' << getDirection() << ' ' << (Np == 0 ? (unsigned long long)getPower() : Np);
//    return ss.str();
//}
//
//string IsotropicPointSourceDescription::timos_str(unsigned long long Np) const
//{
//    stringstream ss;
//    ss << "1 " << plainwhite << getOrigin() << ' ' << (Np == 0 ? (unsigned long long)getPower() : Np);
//    return ss.str();
//}
//
//string VolumeSourceDescription::timos_str(unsigned long long Np) const
//{
//    stringstream ss;
//    ss << "2 " << IDt << ' ' << (Np == 0 ? (unsigned long long) getPower() : Np);
//    return ss.str();
//}
//
//string FaceSourceDescription::timos_str(unsigned long long Np) const
//{
//    stringstream ss;
//    ss << "12 " << f[0] << ' ' << f[1] << ' ' << f[2] << ' ' << (Np == 0 ? (unsigned long long) getPower() : Np);
//    return ss.str();
//}
//
//string SourceMultiDescription::timos_str(unsigned long long Npacket) const
//{
//    string str;
//    for(vector<SourceDescription*>::const_iterator it=sources.begin(); it != sources.end(); ++it)
//        str += (*it)->timos_str() + "\n";
//    return str;
//}

ostream& BallSourceDescription::print(ostream& os) const
{
	return os << "Ball of radius " << r_ << " centred at " << p0_ << " with weight " << getPower();
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


ostream& SourceMultiDescription::print(ostream& os) const
{
    for(vector<SourceDescription*>::const_iterator it=sources.begin(); it != sources.end(); ++it)
        (*it)->print(os) << endl;
    return os;
}


SourceDescription* parse_string(const string& s)
{
	stringstream ss(s);

	char tmp[s.size()];
	double w;

	ss.get(tmp,s.size(),':');
	tmp[ss.gcount()]='\0';

	ss.ignore(1,':');
	ss >> w;

	ss.ignore(1,':');

	SourceDescription* src=NULL;
	string type(tmp);

	if (type=="tet")
	{
		unsigned IDt;
		ss >> IDt;
		if (!ss.fail() && ss.eof())
			src = new VolumeSourceDescription(IDt,w);
	}
	else if(type=="line")
	{
		Point<3,double> a,b;
		ss >> a >> b;
		if (!ss.fail() && ss.eof())
			src = new LineSourceDescription(a,b,w);
	}
	else if(type=="point")
	{
		Point<3,double> p;
		ss >> p;
		if (!ss.fail() && ss.eof())
			src = new IsotropicPointSourceDescription(p,w);
	}
	else if (type=="ball")
	{
		Point<3,double> p;
		double r;

		ss >> p >> r;
		if (!ss.fail() && ss.eof())
			src = new BallSourceDescription(p,r,w);
	}
	else if(type=="pencil")
	{
		Point<3,double> p;
		UnitVector<3,double> dir;
		ss >> p >> dir;
		if (!ss.fail() && ss.eof())
			src = new PencilBeamSourceDescription(p,dir,w);
	}
	else
		cerr << "Invalid source type in string: \"" << s << '"' << endl;

	if (!src)
		cerr << "Source of type " << type << " failed to parse: \"" << s << '"' << endl;

	return src;
}
